#include "webcc/logger.h"

#include <cassert>
#include <cstdarg>
#include <ctime>
#include <iomanip>  // for put_time
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>

#ifdef _WIN32
#include <Windows.h>
#else
// For getting thread ID.
#include <unistd.h>
#include <sys/syscall.h>  // For SYS_xxx definitions
#include <sys/types.h>
#endif  // _WIN32

// Use boost::asio::chrono instead of std::chrono or boost::chrono so that the
// chrono library could be controled by defining BOOST_ASIO_DISABLE_STD_CHRONO
// or not.
#include "boost/asio/detail/chrono.hpp"

namespace webcc {

namespace sfs = std::filesystem;

// -----------------------------------------------------------------------------

static std::string g_main_thread_id;

static const char* kLevelNames[] = {
  "VERB", "INFO", "USER", "WARN", "ERRO"
};

// -----------------------------------------------------------------------------

static FILE* FOpen(const sfs::path& path, bool overwrite) {
#ifdef _WIN32
  return _wfopen(path.wstring().c_str(), overwrite ? L"w+" : L"a+");
#else
  return fopen(path.string().c_str(), overwrite ? "w+" : "a+");
#endif  // _WIN32
}

struct Logger {
  Logger() = default;

  ~Logger() {
    if (file != nullptr) {
      fclose(file);
    }
  }

  void Init(const sfs::path& path, int _modes) {
    modes = _modes;

    // Create log file only if necessary.
    if ((modes & LOG_FILE) != 0 && !path.empty()) {
      file = FOpen(path, (modes & LOG_OVERWRITE) != 0);
    }
  }

  FILE* file = nullptr;
  int modes = 0;
  std::mutex mutex;
};

// Global logger.
static Logger g_logger;

// -----------------------------------------------------------------------------

// The colors related code was adapted from Loguru. See:
//   https://github.com/emilk/loguru/blob/master/loguru.cpp
// Thanks to Loguru!

static const bool g_terminal_has_color = []() {
#ifdef _WIN32
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
  // NOTE: Need Windows 10.
  HANDLE h_output = GetStdHandle(STD_OUTPUT_HANDLE);
  if (h_output != INVALID_HANDLE_VALUE) {
    DWORD mode = 0;
    GetConsoleMode(h_output, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    return SetConsoleMode(h_output, mode) != 0;
  }
  return false;
#else
  const char* term = getenv("TERM");
  if (term == nullptr) {
    return false;
  }

  return strcmp(term, "cygwin") == 0 || strcmp(term, "linux") == 0 ||
         strcmp(term, "rxvt-unicode-256color") == 0 ||
         strcmp(term, "screen") == 0 || strcmp(term, "screen-256color") == 0 ||
         strcmp(term, "screen.xterm-256color") == 0 ||
         strcmp(term, "tmux-256color") == 0 || strcmp(term, "xterm") == 0 ||
         strcmp(term, "xterm-256color") == 0 ||
         strcmp(term, "xterm-termite") == 0 || strcmp(term, "xterm-color") == 0;
#endif
}();

#ifdef _WIN32
#define VTSEQ(ID) ("\x1b[1;" #ID "m")
#else
#define VTSEQ(ID) ("\x1b[" #ID "m")
#endif

// Colors
#define TERM_BLACK        VTSEQ(30)
#define TERM_RED          VTSEQ(31)
#define TERM_GREEN        VTSEQ(32)
#define TERM_YELLOW       VTSEQ(33)
#define TERM_BLUE         VTSEQ(34)
#define TERM_PURPLE       VTSEQ(35)
#define TERM_CYAN         VTSEQ(36)
#define TERM_LLIGHT_GRAY  VTSEQ(37)
#define TERM_LLIGHT_RED   VTSEQ(91)
#define TERM_DIM          VTSEQ(2)

// Formating
#define TERM_BOLD         VTSEQ(1)
#define TERM_UNDERLINE    VTSEQ(4)

// You should end each line with this!
#define TERM_RESET        VTSEQ(0)

// -----------------------------------------------------------------------------

// std::this_thread::get_id() returns a very long ID (same as pthread_self())
// on Linux, e.g., 140219133990656. syscall(SYS_gettid) is much prefered because
// it's shorter and the same as `ps -T -p <pid>` output.
static std::string DoGetThreadID() {
#ifdef _WIN32
  auto thread_id = std::this_thread::get_id();
  std::ostringstream ss;
  ss << thread_id;
  return ss.str();
#else
  return std::to_string(syscall(SYS_gettid));
#endif
}

static std::string GetThreadID() {
  std::string thread_id = DoGetThreadID();
  if (thread_id == g_main_thread_id) {
    return "main";
  }
  return thread_id;
}

static sfs::path InitLogPath(const sfs::path& dir) {
  if (dir.empty()) {
    return sfs::current_path() / WEBCC_LOG_FILE_NAME;
  }

  std::error_code ec;
  if (!sfs::exists(dir, ec) || !sfs::is_directory(dir, ec)) {
    if (!sfs::create_directories(dir, ec) || ec) {
      return {};
    }
  }

  return (dir / WEBCC_LOG_FILE_NAME);
}

void LogInit(const sfs::path& dir, int modes) {
  // Suppose this is called from the main thread.
  g_main_thread_id = DoGetThreadID();

  if ((modes & LOG_FILE) != 0) {
    g_logger.Init(InitLogPath(dir), modes);
  } else {
    g_logger.Init({}, modes);
  }
}

// TODO: Use a fixed static buffer to avoid string reallocations.
static std::string GetTimestamp() {
  using system_clock_t = boost::asio::chrono::system_clock;
  using milliseconds_t = boost::asio::chrono::milliseconds;

  auto now = system_clock_t::now();
  std::time_t t = system_clock_t::to_time_t(now);

  std::ostringstream ss;
  ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
  ss << ".";

  auto milliseconds = boost::asio::chrono::duration_cast<milliseconds_t>(
      now.time_since_epoch());
  ss << std::setfill('0') << std::setw(3) << (milliseconds.count() % 1000);

  return ss.str();
}

void Log(int level, const char* file, int line, const char* format, ...) {
  assert(format != nullptr);

  std::string timestamp = GetTimestamp();
  std::string thread_id = GetThreadID();

  if ((g_logger.modes & LOG_FILE) != 0 && g_logger.file != nullptr) {
    std::lock_guard<std::mutex> lock(g_logger.mutex);

    va_list args;
    va_start(args, format);

    fprintf(g_logger.file, "%s, %s, %7s, %25s, %4d, ",
            timestamp.c_str(), kLevelNames[level], thread_id.c_str(),
            file, line);

    vfprintf(g_logger.file, format, args);

    fprintf(g_logger.file, "\n");

    if ((g_logger.modes & LOG_FLUSH) != 0) {
      fflush(g_logger.file);
    }

    va_end(args);
  }

  if ((g_logger.modes & LOG_CONSOLE) != 0) {
    std::lock_guard<std::mutex> lock(g_logger.mutex);

    va_list args;
    va_start(args, format);

    if (g_terminal_has_color) {
      if (level < WEBCC_WARN) {
        fprintf(stderr, "%s%s, %s, %7s, %25s, %4d, ",
                TERM_RESET,
                timestamp.c_str(), kLevelNames[level], thread_id.c_str(),
                file, line);
      } else {
        fprintf(stderr, "%s%s%s, %s, %7s, %25s, %4d, ",
                TERM_RESET,
                level == WEBCC_WARN ? TERM_YELLOW : TERM_RED,
                timestamp.c_str(), kLevelNames[level], thread_id.c_str(),
                file, line);
      }

      vfprintf(stderr, format, args);

      fprintf(stderr, "%s\n", TERM_RESET);
    } else {
      fprintf(stderr, "%s, %s, %7s, %25s, %4d, ",
              timestamp.c_str(), kLevelNames[level], thread_id.c_str(),
              file, line);

      vfprintf(stderr, format, args);

      fprintf(stderr, "\n");
    }

    if ((g_logger.modes & LOG_FLUSH) != 0) {
      fflush(stderr);
    }

    va_end(args);
  }
}

}  // namespace webcc

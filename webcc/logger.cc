#include "webcc/logger.h"

#if WEBCC_ENABLE_LOG

#include <cassert>
#include <chrono>
#include <cstdarg>
#include <ctime>
#include <iomanip>  // for put_time
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#if (defined(_WIN32) || defined(_WIN64))
#include <Windows.h>
#else
// For getting thread ID.
#include <sys/syscall.h>
#include <sys/types.h>
#endif

#include "boost/filesystem.hpp"

namespace webcc {

// -----------------------------------------------------------------------------

static std::string g_main_thread_id;

static const char* kLevelNames[] = {
  "VERB", "INFO", "WARN", "ERRO", "FATA"
};

// -----------------------------------------------------------------------------

struct Logger {
  Logger() : file(nullptr), modes(0) {
  }

  void Init(const std::string& path, int _modes) {
    modes = _modes;

    // Create log file only if necessary.
    if ((modes & LOG_FILE) != 0 && !path.empty()) {
      if ((modes & LOG_OVERWRITE) != 0) {
        file = fopen(path.c_str(), "w+");
      } else {
        // Append to existing file.
        file = fopen(path.c_str(), "a+");
      }
    }
  }

  ~Logger() {
    if (file != nullptr) {
      fclose(file);
    }
  }

  FILE* file;
  int modes;
  std::mutex mutex;
};

// Global logger.
static Logger g_logger;

// -----------------------------------------------------------------------------

// The colors related code was adapted from Loguru. See:
//   https://github.com/emilk/loguru/blob/master/loguru.cpp
// Thanks to Loguru!

bool g_colorlogtostderr = true;

static const bool g_terminal_has_color = []() {
#if (defined(_WIN32) || defined(_WIN64))
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

// Colors

#if (defined(_WIN32) || defined(_WIN64))
#define VTSEQ(ID) ("\x1b[1;" #ID "m")
#else
#define VTSEQ(ID) ("\x1b[" #ID "m")
#endif

const char* TerminalBlack() {
  return g_terminal_has_color ? VTSEQ(30) : "";
}
const char* TerminalRed() {
  return g_terminal_has_color ? VTSEQ(31) : "";
}
const char* TerminalGreen() {
  return g_terminal_has_color ? VTSEQ(32) : "";
}
const char* TerminalYellow() {
  return g_terminal_has_color ? VTSEQ(33) : "";
}
const char* TerminalBlue() {
  return g_terminal_has_color ? VTSEQ(34) : "";
}
const char* TerminalPurple() {
  return g_terminal_has_color ? VTSEQ(35) : "";
}
const char* TerminalCyan() {
  return g_terminal_has_color ? VTSEQ(36) : "";
}
const char* TerminalLightGray() {
  return g_terminal_has_color ? VTSEQ(37) : "";
}
const char* TerminalWhite() {
  return g_terminal_has_color ? VTSEQ(37) : "";
}
const char* TerminalLightRed() {
  return g_terminal_has_color ? VTSEQ(91) : "";
}
const char* TerminalDim() {
  return g_terminal_has_color ? VTSEQ(2) : "";
}

// Formating
const char* TerminalBold() {
  return g_terminal_has_color ? VTSEQ(1) : "";
}
const char* TerminalUnderline() {
  return g_terminal_has_color ? VTSEQ(4) : "";
}

// You should end each line with this!
const char* TerminalReset() {
  return g_terminal_has_color ? VTSEQ(0) : "";
}

// -----------------------------------------------------------------------------

namespace bfs = boost::filesystem;

// std::this_thread::get_id() returns a very long ID (same as pthread_self())
// on Linux, e.g., 140219133990656. syscall(SYS_gettid) is much prefered because
// it's shorter and the same as `ps -T -p <pid>` output.
static std::string DoGetThreadID() {
#if (defined(_WIN32) || defined(_WIN64))
  auto thread_id = std::this_thread::get_id();
  std::stringstream ss;
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

static bfs::path InitLogPath(const std::string& dir) {
  if (dir.empty()) {
    return bfs::current_path() / WEBCC_LOG_FILE_NAME;
  }

  bfs::path path = bfs::path(dir);
  if (!bfs::exists(path) || !bfs::is_directory(path)) {
    boost::system::error_code ec;
    if (!bfs::create_directories(path, ec) || ec) {
      return bfs::path();
    }
  }

  path /= WEBCC_LOG_FILE_NAME;
  return path;
}

void LogInit(const std::string& dir, int modes) {
  if ((modes & LOG_FILE) != 0) {
    bfs::path path = InitLogPath(dir);
    g_logger.Init(path.string(), modes);
  } else {
    g_logger.Init("", modes);
  }

  // Suppose LogInit() is called from the main thread.
  g_main_thread_id = DoGetThreadID();
}

static std::string GetTimestamp() {
  using system_clock = std::chrono::system_clock;
  using milliseconds = std::chrono::milliseconds;

  auto now = system_clock::now();
  std::time_t t = system_clock::to_time_t(now);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");

  milliseconds milli_seconds =
      std::chrono::duration_cast<milliseconds>(now.time_since_epoch());
  std::string micro_seconds_str = std::to_string(milli_seconds.count() % 1000);
  while (micro_seconds_str.size() < 3) {
    micro_seconds_str = "0" + micro_seconds_str;
  }

  ss << "." << micro_seconds_str;

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

    fprintf(g_logger.file, "%s, %s, %7s, %20s, %4d, ",
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

    if (g_colorlogtostderr && g_terminal_has_color) {
      if (level < WEBCC_WARN) {
        fprintf(stderr, "%s%s, %s, %7s, %20s, %4d, ",
                TerminalReset(),
                timestamp.c_str(), kLevelNames[level], thread_id.c_str(),
                file, line);
      } else {
        fprintf(stderr, "%s%s%s, %s, %7s, %20s, %4d, ",
                TerminalReset(),
                level == WEBCC_WARN ? TerminalYellow() : TerminalRed(),
                timestamp.c_str(), kLevelNames[level], thread_id.c_str(),
                file, line);
      }

      vfprintf(stderr, format, args);

      fprintf(stderr, "%s\n", TerminalReset());
    } else {
      fprintf(stderr, "%s, %s, %7s, %20s, %4d, ",
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

#endif  // WEBCC_ENABLE_LOG

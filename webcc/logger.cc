#include "webcc/logger.h"

#if WEBCC_ENABLE_LOG

#include <cassert>
#include <chrono>
#include <cstdarg>
#include <ctime>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include "boost/filesystem.hpp"

namespace webcc {

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

static std::thread::id g_main_thread_id;

static const char* kLevelNames[] = {
  "VERB", "INFO", "WARN", "ERRO", "FATA"
};

namespace bfs = boost::filesystem;

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
  g_main_thread_id = std::this_thread::get_id();
}

static std::string GetTimestamp() {
  using namespace std::chrono;

  auto now = system_clock::now();
  std::time_t now_c = system_clock::to_time_t(now);
  std::tm* now_tm = std::localtime(&now_c);

  char buf[20];
  std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", now_tm);

  std::string timestamp(buf);

  milliseconds milli_seconds = duration_cast<milliseconds>(
      now.time_since_epoch());
  std::string micro_seconds_str = std::to_string(milli_seconds.count() % 1000);
  while (micro_seconds_str.size() < 3) {
    micro_seconds_str = "0" + micro_seconds_str;
  }

  timestamp.append(".");
  timestamp.append(micro_seconds_str);

  return timestamp;
}

static std::string GetThreadID() {
  std::thread::id thread_id = std::this_thread::get_id();

  if (thread_id == g_main_thread_id) {
    return "main";
  }

  std::stringstream ss;
  ss << thread_id;
  return ss.str();
}

static void WriteToFile(FILE* fd, int level, const char* file, int line,
                        const char* format, va_list args) {
  std::lock_guard<std::mutex> lock(g_logger.mutex);

  fprintf(fd, "%s, %s, %7s, %24s, %4d, ",
          GetTimestamp().c_str(), kLevelNames[level], GetThreadID().c_str(),
          file, line);

  vfprintf(fd, format, args);

  fprintf(fd, "\n");

  if ((g_logger.modes & LOG_FLUSH) != 0) {
    fflush(fd);
  }
}

void LogWrite(int level, const char* file, int line, const char* format, ...) {
  assert(format != nullptr);

  va_list args;
  va_start(args, format);

  if ((g_logger.modes & LOG_FILE) != 0 && g_logger.file != nullptr) {
    WriteToFile(g_logger.file, level, file, line, format, args);
  }

  if ((g_logger.modes & LOG_CONSOLE) != 0) {
    WriteToFile(stderr, level, file, line, format, args);
  }

  va_end(args);
}

}  // namespace webcc

#endif  // WEBCC_ENABLE_LOG

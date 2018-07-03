#include "webcc/logger.h"

#if WEBCC_ENABLE_LOG

#include <cassert>
#include <chrono>
#include <cstdarg>
#include <ctime>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include "boost/filesystem.hpp"

namespace webcc {

struct Logger {
  Logger(const std::string& path, int modes) : file(nullptr), modes(modes) {
    if (!path.empty()) {
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
static std::shared_ptr<Logger> g_logger;

static std::thread::id g_main_thread_id;

static const char* kLevelNames[] = {
  "VERB", "INFO", "WARN", "ERRO", "FATA"
};

namespace bfs = boost::filesystem;

static bfs::path InitLogPath(const std::string& dir) {
  if (dir.empty()) {
    return bfs::current_path() / WEBCC_LOG_FILE;
  }

  bfs::path path = bfs::path(dir);
  if (!bfs::exists(path) || !bfs::is_directory(path)) {
    boost::system::error_code ec;
    if (!bfs::create_directories(path, ec) || ec) {
      return bfs::path();
    }
  }

  path /= WEBCC_LOG_FILE;
  return path;
}

void LogInit(const std::string& dir, int modes) {
  bfs::path path = InitLogPath(dir);
  g_logger.reset(new Logger(path.string(), modes));

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

void LogWrite(int level, const char* file, int line, const char* format, ...) {
  assert(format != nullptr);

  va_list va_ptr_file;
  va_list va_ptr_console;

  va_start(va_ptr_file, format);
  va_start(va_ptr_console, format);

  if ((g_logger->modes & LOG_FILE) != 0 && g_logger->file != nullptr) {
    std::lock_guard<std::mutex> lock(g_logger->mutex);

    fprintf(g_logger->file, "%s, %s, %5s, %24s, %4d, ",
            GetTimestamp().c_str(), kLevelNames[level], GetThreadID().c_str(),
            file, line);

    vfprintf(g_logger->file, format, va_ptr_console);

    fprintf(g_logger->file, "\n");

    if ((g_logger->modes & LOG_FLUSH) != 0) {
      fflush(g_logger->file);
    }
  }

  if ((g_logger->modes & LOG_CONSOLE) != 0) {
    std::lock_guard<std::mutex> lock(g_logger->mutex);

    fprintf(stderr, "%s, %s, %5s, %24s, %4d, ",
            GetTimestamp().c_str(), kLevelNames[level], GetThreadID().c_str(),
            file, line);

    vfprintf(stderr, format, va_ptr_console);
    fprintf(stderr, "\n");

    if ((g_logger->modes & LOG_FLUSH) != 0) {
      fflush(stderr);
    }
  }

  va_end(va_ptr_file);
  va_end(va_ptr_console);
}

}  // namespace webcc

#endif  // WEBCC_ENABLE_LOG

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

namespace webcc {

struct Logger {
  int modes;
  std::mutex mutex;
};

// Global logger.
static Logger g_logger{ 0 };

static std::thread::id g_main_thread_id;

static const char* kLevelNames[] = {
  "VERB", "INFO", "WARN", "ERRO", "FATA"
};

void LogInit(int modes) {
  g_logger.modes = modes;

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

  std::lock_guard<std::mutex> lock(g_logger.mutex);

  va_list va_ptr_console;
  va_start(va_ptr_console, format);

  fprintf(stderr, "%s, %s, %5s, %24s, %4d, ",
          GetTimestamp().c_str(), kLevelNames[level], GetThreadID().c_str(),
          file, line);
  vfprintf(stderr, format, va_ptr_console);
  fprintf(stderr, "\n");

  if ((g_logger.modes & FLUSH) == FLUSH) {
    fflush(stderr);
  }

  va_end(va_ptr_console);
}

}  // namespace webcc

#endif  // WEBCC_ENABLE_LOG

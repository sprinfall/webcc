#include "webcc/logger.h"

#if WEBCC_ENABLE_LOG

#include <cassert>
#include <cstdarg>
#include <mutex>  // NOLINT
#include <string>

#include "boost/thread.hpp"  // For thread ID.

namespace webcc {

static std::string GetThreadId() {
  boost::thread::id thread_id = boost::this_thread::get_id();
  std::stringstream ss;
  ss << thread_id;
  return ss.str();
}

static const char* kLevelNames[] = {
  "VERB", "INFO", "WARN", "ERRO", "FATA"
};

struct Logger {
  int level;
  int modes;
  std::mutex mutex;
};

// Global logger.
static Logger g_logger{ VERB };

void LogInit(int level, int modes) {
  g_logger.modes = modes;
  g_logger.level = level;
}

void LogWrite(int level, const char* file, int line, const char* format, ...) {
  assert(format != nullptr);

  if (g_logger.level > level) {
    return;
  }

  std::lock_guard<std::mutex> lock(g_logger.mutex);

  va_list va_ptr_console;
  va_start(va_ptr_console, format);

  fprintf(stderr,
          "%s, %5s, %24s, %4d, ",
          kLevelNames[level],
          GetThreadId().c_str(),
          file,
          line);
  vfprintf(stderr, format, va_ptr_console);
  fprintf(stderr, "\n");

  if ((g_logger.modes & FLUSH) == FLUSH) {
    fflush(stderr);
  }

  va_end(va_ptr_console);
}

}  // namespace webcc

#endif  // WEBCC_ENABLE_LOG

#ifndef WEBCC_LOGGER_H_
#define WEBCC_LOGGER_H_

// This file was generated from "config.h.in" by CMake.
#include "webcc/config.h"

#ifdef WEBCC_ENABLE_LOG

#include <cstring>  // for strrchr()
#include <string>

#include "boost/filesystem/path.hpp"

// Log levels.
// VERB is similar to DEBUG commonly used by other projects.
// USER is for the users who want to log their own logs but don't want any
// VERB or INFO.
#define WEBCC_VERB 0
#define WEBCC_INFO 1
#define WEBCC_USER 2
#define WEBCC_WARN 3
#define WEBCC_ERRO 4

// Default log level.
#ifndef WEBCC_LOG_LEVEL
#define WEBCC_LOG_LEVEL WEBCC_USER
#endif

#define WEBCC_LOG_FILE_NAME "webcc.log"

namespace webcc {

enum LogMode {
  LOG_FILE        = 1,  // Log to file.
  LOG_CONSOLE     = 2,  // Log to console.
  LOG_FLUSH       = 4,  // Flush on each log.
  LOG_OVERWRITE   = 8,  // Overwrite any existing log file.
};

// Commonly used modes.
const int LOG_CONSOLE_FILE_APPEND = LOG_CONSOLE | LOG_FILE;
const int LOG_CONSOLE_FILE_OVERWRITE = LOG_CONSOLE | LOG_FILE | LOG_OVERWRITE;
const int LOG_FILE_OVERWRITE = LOG_FILE | LOG_OVERWRITE;

// Initialize logger.
// If |dir| is empty, log file will be generated in current directory.
void LogInit(const boost::filesystem::path& dir, int modes);

void Log(int level, const char* file, int line, const char* format, ...);

}  // namespace webcc

// Initialize the logger with a level.
#define WEBCC_LOG_INIT(dir, modes) webcc::LogInit(dir, modes);

// Definition of _WIN32 & _WIN64:
//   https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=vs-2015
#if (defined(_WIN32) || defined(_WIN64))

// See: https://stackoverflow.com/a/8488201
// ISSUE: The last path separator of __FILE__ in a header file becomes "/"
//        instead of "\". The result is that __FILENAME__ will contain a
//        prefix of "webcc/". So don't log from a header file!
#define __FILENAME__ std::strrchr("\\" __FILE__, '\\') + 1

#else

#define __FILENAME__ std::strrchr("/" __FILE__, '/') + 1

#endif  // defined(_WIN32) || defined(_WIN64)

#if WEBCC_LOG_LEVEL <= WEBCC_VERB
#define LOG_VERB(format, ...) \
  webcc::Log(WEBCC_VERB, __FILENAME__, __LINE__, format, ##__VA_ARGS__);
#else
#define LOG_VERB(format, ...)
#endif

#if WEBCC_LOG_LEVEL <= WEBCC_INFO
#define LOG_INFO(format, ...) \
  webcc::Log(WEBCC_INFO, __FILENAME__, __LINE__, format, ##__VA_ARGS__);
#else
#define LOG_INFO(format, ...)
#endif

#if WEBCC_LOG_LEVEL <= WEBCC_USER
#define LOG_USER(format, ...) \
  webcc::Log(WEBCC_USER, __FILENAME__, __LINE__, format, ##__VA_ARGS__);
#else
#define LOG_INFO(format, ...)
#endif

#if WEBCC_LOG_LEVEL <= WEBCC_WARN
#define LOG_WARN(format, ...) \
  webcc::Log(WEBCC_WARN, __FILENAME__, __LINE__, format, ##__VA_ARGS__);
#else
#define LOG_WARN(format, ...)
#endif

#if WEBCC_LOG_LEVEL <= WEBCC_ERRO
#define LOG_ERRO(format, ...) \
  webcc::Log(WEBCC_ERRO, __FILENAME__, __LINE__, format, ##__VA_ARGS__);
#else
#define LOG_ERRO(format, ...)
#endif

#else  // WEBCC_ENABLE_LOG == 0

#define WEBCC_LOG_INIT(dir, modes)

#define LOG_VERB(format, ...)
#define LOG_INFO(format, ...)
#define LOG_USER(format, ...)
#define LOG_WARN(format, ...)
#define LOG_ERRO(format, ...)

#endif  // WEBCC_ENABLE_LOG

#endif  // WEBCC_LOGGER_H_

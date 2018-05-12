#ifndef WEBCC_LOGGER_H_
#define WEBCC_LOGGER_H_

// Simple console logger.

#if WEBCC_ENABLE_LOG

#include <cstring>  // for strrchr()

namespace webcc {

enum LogLevel {
  VERB = 0,
  INFO,
  WARN,
  ERRO,
  FATA,
};

enum LogMode {
  FLUSH = 1,
};

void LogInit(int level, int modes);

void LogWrite(int level, const char* file, int line, const char* format, ...);

}  // namespace webcc

// Initialize the logger with a level.
// E.g., LOG_INIT(ERRO, FLUSH)
#define LOG_INIT(level, modes) webcc::LogInit(level, modes);

#if (defined(WIN32) || defined(_WIN64))

// See: https://stackoverflow.com/a/8488201
#define __FILENAME__ std::strrchr("\\" __FILE__, '\\') + 1

#define LOG_VERB(format, ...) \
    webcc::LogWrite(webcc::VERB, __FILENAME__, __LINE__, format, ##__VA_ARGS__);

#define LOG_INFO(format, ...) \
    webcc::LogWrite(webcc::INFO, __FILENAME__, __LINE__, format, ##__VA_ARGS__);

#define LOG_WARN(format, ...) \
    webcc::LogWrite(webcc::WARN, __FILENAME__, __LINE__, format, ##__VA_ARGS__);

#define LOG_ERRO(format, ...) \
    webcc::LogWrite(webcc::ERRO, __FILENAME__, __LINE__, format, ##__VA_ARGS__);

#define LOG_FATA(format, ...) \
    webcc::LogWrite(webcc::FATA, __FILENAME__, __LINE__, format, ##__VA_ARGS__);

#else

// See: https://stackoverflow.com/a/8488201
#define __FILENAME__ std::strrchr("/" __FILE__, '/') + 1

#define LOG_VERB(format, args...) \
    webcc::LogWrite(webcc::VERB, __FILENAME__, __LINE__, format, ##args);

#define LOG_INFO(format, args...) \
    webcc::LogWrite(webcc::INFO, __FILENAME__, __LINE__, format, ##args);

#define LOG_WARN(format, args...) \
    webcc::LogWrite(webcc::WARN, __FILENAME__, __LINE__, format, ##args);

#define LOG_ERRO(format, args...) \
    webcc::LogWrite(webcc::ERRO, __FILENAME__, __LINE__, format, ##args);

#define LOG_FATA(format, args...) \
    webcc::LogWrite(webcc::FATA, __FILENAME__, __LINE__, format, ##args);

#endif  // defined(WIN32) || defined(_WIN64)

#else  // WEBCC_ENABLE_LOG == 0

#define LOG_INIT(level, modes)

#if (defined(WIN32) || defined(_WIN64))
#define LOG_VERB(format, ...)
#define LOG_INFO(format, ...)
#define LOG_WARN(format, ...)
#define LOG_ERRO(format, ...)
#define LOG_FATA(format, ...)
#else
#define LOG_VERB(format, args...)
#define LOG_INFO(format, args...)
#define LOG_WARN(format, args...)
#define LOG_ERRO(format, args...)
#define LOG_FATA(format, args...)
#endif  // defined(WIN32) || defined(_WIN64)

#endif  // WEBCC_ENABLE_LOG

#endif  // WEBCC_LOGGER_H_

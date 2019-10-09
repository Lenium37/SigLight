//
// Created by Jan on 21.05.2019.
//

#ifndef RASPITOLIGHT_SRC_CORE_LIB_LOGGER_H_
#define RASPITOLIGHT_SRC_CORE_LIB_LOGGER_H_


#if defined(_WIN32) || defined(WIN32)
#include <mingw.mutex.h>
#include <mingw.condition_variable.h>
#include <mingw.thread.h>
#endif

#include <spdlog/spdlog.h>

/*! \brief Convinience Wrapper around the used logging library.
 *         Currently https://github.com/gabime/spdlog is used
 *
 *  Wrapper around spdlog, so we can:
 * - centralice logging configuration
 * - easily change logging library in the future
 * Example:
 * \code{.cpp}
 * #include <logger.h>
 *
   Logger::info("test: {}", 42); // easy formatting, see https://github.com/fmtlib/fmt
   Logger::debug("We should not see this");
   Logger::set_level(Logger::LevelEnum::debug);
   Logger::debug("We should see this");
  \endcode
 */
class Logger {
 public:

  enum class LevelEnum {
    trace = 0,
    debug = 1,
    info = 2,
    warn = 3,
    err = 4,
    critical = 5,
    off = 6
  };
  static void set_level(LevelEnum level);

  template<typename... Args>
  static void trace(const char *fmt, const Args &... args);

  template<typename... Args>
  static void debug(const char *fmt, const Args &... args);

  template<typename... Args>
  static void info(const char *fmt, const Args &... args);

  template<typename... Args>
  static void warning(const char *fmt, const Args &... args);

  template<typename... Args>
  static void error(const char *fmt, const Args &... args);

  template<typename... Args>
  static void critical(const char *fmt, const Args &... args);

  template<typename T>
  static void trace(const T &msg);

  template<typename T>
  static void debug(const T &msg);

  template<typename T>
  static void info(const T &msg);

  template<typename T>
  static void warning(const T &msg);

  template<typename T>
  static void critical(const T &msg);
};

template<typename... Args>
void Logger::trace(const char *fmt, const Args &... args) {
  spdlog::trace(fmt, args...);
}

template<typename... Args>
void Logger::debug(const char *fmt, const Args &... args) {
  spdlog::debug(fmt, args...);
}

template<typename... Args>
void Logger::info(const char *fmt, const Args &... args) {
  spdlog::info(fmt, args...);
}

template<typename... Args>
void Logger::warning(const char *fmt, const Args &... args) {
  spdlog::warn(fmt, args...);
}

template<typename... Args>
void Logger::error(const char *fmt, const Args &... args) {
  spdlog::error(fmt, args...);
}

template<typename... Args>
void Logger::critical(const char *fmt, const Args &... args) {
  spdlog::critical(fmt, args...);
}
template<typename T>
void Logger::trace(const T &msg) {
  spdlog::trace(msg);
}

template<typename T>
void Logger::debug(const T &msg) {
  spdlog::debug(msg);
}

template<typename T>
void Logger::info(const T &msg) {
  spdlog::info(msg);
}

template<typename T>
void Logger::warning(const T &msg) {
  spdlog::warn(msg);
}

template<typename T>
void Logger::critical(const T &msg) {
  spdlog::critical(msg);
}

#endif //RASPITOLIGHT_SRC_CORE_LIB_LOGGER_H_

#pragma once

#include <cstdarg>
#include <cstdio>

class Logger {

public:
  enum Level { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

  static void setLevel(Level lvl);

  // The numbers (1, 2) mean:
  // 1: The format string is the 1st argument
  // 2: The variadic arguments start at the 2nd position
  static void debug(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
  static void info(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
  static void warn(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
  static void error(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
  static void fatal(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
  static void fatalWith(int exitCode, const char *fmt, ...)
      __attribute__((format(printf, 2, 3)));

private:
  static Level minLevel;

  static void log(FILE *writer, Level lvl, const char *fmt, va_list args);
};

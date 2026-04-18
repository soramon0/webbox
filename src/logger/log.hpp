#pragma once

#include <cstdarg>
#include <cstdio>

class Logger {

public:
  enum Level { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

  static void setLevel(Level lvl);
  static void debug(const char *fmt, ...);
  static void info(const char *fmt, ...);
  static void warn(const char *fmt, ...);
  static void error(const char *fmt, ...);
  static void fatal(const char *fmt, ...);
  static void fatalWith(int exitCode, const char *fmt, ...);

private:
  static Level minLevel;

  static void log(FILE *writer, Level lvl, const char *fmt, va_list args);
};

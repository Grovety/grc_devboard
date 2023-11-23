#include "Log.hpp"
#include <stdarg.h>
#include <stdio.h>

void _log(int8_t level, const char *tag, const char *fmt, ...) {
  if (level >= _LOG_LEVEL_NUM)
    return;

  if (level >= LOG_LEVEL) {
    va_list argp;
    va_start(argp, fmt);

    printf("%s: ", tag);
    vprintf(fmt, argp);
    printf("\n");

    va_end(argp);
  }
}

void _log(const char *fmt, ...) {
  if (LOG_LEVEL >= LOG_LEVEL_NONE)
    return;

  va_list argp;
  va_start(argp, fmt);

  vprintf(fmt, argp);
  printf("\n");

  va_end(argp);
}

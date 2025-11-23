#include <stdarg.h>
#include <stdio.h>

#include "log.h"

void
l(const char* fmt, ...)
{
  const char* log_fmt = "LOG: %s";
  char buf[1024];

  va_list args;
  va_start(args, fmt);

  if (vsnprintf(buf, 1024, log_fmt, args) < 0) {
    perror("log");

  }

  va_end(args);
}

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "config.h"
#include "error.h"

Verbosity g_verbosity = QUIET;

static const char* VERBOSITY[] = {
  [QUIET] = "QUIET",
  [INFO]  = "INFO",
  [DEBUG] = "DBG",
};

void
dbg(const char* fmt, ...)
{
  if (g_verbosity < DEBUG) return;

  char        buf[MESSAGE_BUFFER_SIZE];
  const char* prefix = "DBG: ";
  memcpy(buf, prefix, strlen(prefix));

  va_list args;
  va_start(args, fmt);

  if (vsnprintf(&buf[strlen(prefix)], MESSAGE_BUFFER_SIZE, fmt, args) < 0) {
    handle_error("dbg vsnprintf");
  }

  if (puts(buf) == EOF) {
    handle_error("dbg puts");
  }

  va_end(args);
}

void
msg(const char* fmt, ...)
{
  if (g_verbosity < INFO) return;

  char        buf[MESSAGE_BUFFER_SIZE];
  const char* prefix = "INFO: ";
  memcpy(buf, prefix, strlen(prefix));

  va_list args;
  va_start(args, fmt);

  if (vsnprintf(&buf[strlen(prefix)], MESSAGE_BUFFER_SIZE, fmt, args) < 0) {
    handle_error("msg vsnprintf");
  }

  if (puts(buf) == EOF) {
    handle_error("msg puts");
  }


  va_end(args);
}

const char*
verbosity_to_string(Verbosity v)
{
  return VERBOSITY[v];
}

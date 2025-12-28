#include "log.h"

#include "config.h"
#include "error.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static Verbosity s_verbosity = QUIET;

static const char* VERBOSITY[] = {
   [QUIET] = "QUIET",
   [INFO]  = "INFO",
   [DEBUG] = "DBG",
};

void
dbg (const char* fmt, ...)
{
   if (s_verbosity < DEBUG)
      return;

   char        buf[MESSAGE_BUFFER_SIZE];
   const char* prefix = "DBG: ";

   memcpy (buf, prefix, strlen (prefix));

   va_list args;
   va_start (args, fmt);

   if (vsnprintf (&buf[strlen (prefix)], MESSAGE_BUFFER_SIZE, fmt, args) < 0)
      set_error ("dbg vsnprintf");

   if (puts (buf) == EOF)
      set_error ("dbg puts");

   va_end (args);
}

void
msg (const char* fmt, ...)
{
   if (s_verbosity < INFO)
      return;

   char        buf[MESSAGE_BUFFER_SIZE];
   const char* prefix = "INFO: ";

   memcpy (buf, prefix, strlen (prefix));

   va_list args;
   va_start (args, fmt);

   if (vsnprintf (&buf[strlen (prefix)], MESSAGE_BUFFER_SIZE, fmt, args) < 0)
      set_error ("msg vsnprintf");

   if (puts (buf) == EOF)
      set_error ("msg puts");

   va_end (args);
}

void
set_verbosity (Verbosity verbosity)
{
   s_verbosity = verbosity;
}

const char*
get_verbosity_string ()
{
   return VERBOSITY[s_verbosity];
}

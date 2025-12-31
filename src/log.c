#include "log.h"

#include "config.h"
#include "error.h"
#include "threads.h"

#include <ctype.h>
#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
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

   char* thread_name = strdup (thread_id_to_name (pthread_self ()));
   for (char* pos = thread_name; *pos != '\0'; ++pos)
      *pos = (char)toupper (*pos);

   char        buf[MESSAGE_BUFFER_SIZE];
   const char* prefix[] = { "DBG: ", "[", thread_name, "] " };

   ptrdiff_t offset = 0;
   for (size_t i = 0; i < sizeof (prefix) / sizeof (prefix[0]); ++i)
   {
      memcpy (buf + offset, prefix[i], strlen (prefix[i]));
      offset += strlen (prefix[i]);
   }

   va_list args;
   va_start (args, fmt);

   // TODO: think about how to propagate these errors
   if (vsnprintf (&buf[offset], MESSAGE_BUFFER_SIZE, fmt, args) < 0)
      set_error ("vsnprintf");

   if (puts (buf) == EOF)
      set_error ("puts");

   va_end (args);

   free (thread_name);
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
      set_error ("vsnprintf");

   if (puts (buf) == EOF)
      set_error ("puts");

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

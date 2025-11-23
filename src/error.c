#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "error.h"

void
handle_error(const char* fmt, ...)
{
    char        buf[1024];
    const char* prefix = "ERROR: ";
    memcpy(buf, prefix, strlen(prefix));

    va_list args;
    va_start(args, fmt);

    if (vsnprintf(&buf[strlen(prefix)], 1024, fmt, args) < 0) {
      perror("handle_error vsnprintf");
      kill(getpid(), SIGINT);
    }

    va_end(args);

    perror(buf);
    kill(getpid(), SIGINT);
}

#define _POSIX_C_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#include "error.h"

void
handle_error(const char* fmt, ...)
{
    char buf[256];

    va_list args;
    va_start(args, fmt);

    if (vsnprintf(buf, 256, fmt, args) < 0) {
      perror("handle_error vsnprintf");
      kill(getpid(), SIGINT);
    }

    va_end(args);

    perror(buf);
    kill(getpid(), SIGINT);
}

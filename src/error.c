#define _POSIX_C_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>

#include "error.h"
#include "context.h"
#include "mutex.h"

// TODO: maybe a way to just deinit stuff after an error
void
handle_error(Context* ctx, const char* fmt, ...)
{
    char buf[256];

    va_list args;
    va_start(args, fmt);

    vsnprintf(buf, 256, fmt, args);

    va_end(args);

    perror(buf);

    MUTEX(&ctx->mutex, {
            ctx->should_quit_app = true;
            ctx->has_error = true;
          });

    kill(getpid(), SIGUSR1);
}

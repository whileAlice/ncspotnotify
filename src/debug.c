#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "context.h"
#include "error.h"
#include "mutex.h"

void*
debug_thread(void* args)
{
  Context* ctx = (Context*)args;

  char   buf[1024];
  nfds_t fd_count = 2;

  struct pollfd* poll_fds = calloc(fd_count, sizeof(struct pollfd));
  if (poll_fds == NULL) {
    handle_error(ctx, "calloc (poll_fds)");
  }

  poll_fds[0].fd = STDIN_FILENO;
  poll_fds[1].fd = ctx->debug_pipe_fds[0];
  poll_fds[0].events = poll_fds[1].events = POLLIN;

  while (!ctx->should_quit_app) {
    if (poll(poll_fds, fd_count, -1) == -1) {
      handle_error(ctx, "poll");
    }

    for (nfds_t i = 0; i < fd_count; ++i) {
      if (poll_fds[i].revents == 0) continue;

      if (poll_fds[i].revents & POLLIN) {
        ssize_t length = read(poll_fds[i].fd, buf, sizeof(buf) - 1);
        if (length == -1) {
          handle_error(ctx, "read (poll_fds[%d])", (int)i);
        }
        buf[length - 1] = '\0';

        if (strcmp(buf, "quit") == 0) {
          MUTEX(&ctx->lock, { ctx->should_quit_app = true; });

          break;
        }
      }
    }
  }

  if (close(ctx->debug_pipe_fds[0]) == -1) {
    handle_error(ctx, "close (pipe read end)");
  }


  return NULL;
}

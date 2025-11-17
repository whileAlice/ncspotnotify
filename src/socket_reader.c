#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "socket_reader.h"
#include "context.h"
#include "error.h"
#include "mutex.h"
#include "usleep.h"

#define SOCKET_PATH  "/run/user/1000/ncspot/ncspot.sock"

void*
socket_reader_thread(void* args)
{
  Context* ctx = (Context*)args;
  int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  struct sockaddr_un socket_address;

  socket_address.sun_family = AF_UNIX;
  strncpy(socket_address.sun_path, SOCKET_PATH,
          sizeof(socket_address.sun_path) - 1);

  int err = connect(socket_fd, (const struct sockaddr *)&socket_address,
                    sizeof(socket_address));
  if (err == -1) {
    handle_error(ctx, "connect");
  }

  int flags = fcntl(socket_fd, F_GETFL, 0);
  fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);

  char buf[1024];
  while (!ctx->should_quit_app) {
    ssize_t bytes_read = read(socket_fd, buf, sizeof(buf) - 1);

    if (bytes_read > 0) {
      buf[bytes_read] = '\0';

      MUTEX(&ctx->lock, {
              strcpy(ctx->message, buf);
              ctx->is_message_ready = true;
            });
    } else if (bytes_read == 0) {
      printf("socket was closed\n");

      MUTEX(&ctx->lock, { ctx->should_quit_app = true; });

      if (write(ctx->debug_pipe_fds[1], "quit", 5) != 5) {
        handle_error(ctx, "write (pipe write end)");
      }
      if (close(ctx->debug_pipe_fds[1]) == -1) {
        handle_error(ctx, "close (pipe write end)");
      }
    } else if (errno == EAGAIN) {
      usleep(100000);
    } else {
      handle_error(ctx, "read (socket)");
    }
  }

  printf("closing socket reader gracefully\n");
  if (close(socket_fd) == -1) {
    handle_error(ctx, "close (socket)");
  }

  return NULL;
}

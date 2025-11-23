#include <poll.h>
#include <pthread.h>
#define _POSIX_C_SOURCE
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>

#include "socket_reader.h"
#include "config.h"
#include "context.h"
#include "error.h"
#include "mutex.h"
#include "socket_messages.h"

#define SOCKET_PATH  "/run/user/1000/ncspot/ncspot.sock"

void*
socket_reader_thread(void* args)
{
  Context* ctx = (Context*)args;

  struct sockaddr_un socket_address;
  int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

  socket_address.sun_family = AF_UNIX;
  strncpy(socket_address.sun_path, SOCKET_PATH,
          sizeof(socket_address.sun_path) - 1);

  int err = connect(socket_fd, (const struct sockaddr *)&socket_address,
                    sizeof(socket_address));
  if (err == -1) {
    handle_error("socket_reader_thread connect");

    goto close;
  }

  nfds_t fd_count = 2;
  struct pollfd* poll_fds = calloc(fd_count, sizeof(struct pollfd));
  if (poll_fds == NULL) {
    handle_error("socket_reader_thread calloc (poll_fds)");
  }

  poll_fds[0].fd = socket_fd;
  poll_fds[1].fd = ctx->reader_pipe[0];
  poll_fds[0].events = poll_fds[1].events = POLLIN;

  char buf[SOCKET_BUFFER_SIZE];
  while (true) {
    printf("socket reader waiting for socket message\n");

    if (poll(poll_fds, fd_count, -1) == -1) {
      handle_error("socket_reader_thread poll");
    }

    if (ctx->should_quit_app) {
      break;
    }

    for (nfds_t i = 0; i < fd_count; ++i) {
      if (poll_fds[i].revents == 0) continue;

      if (poll_fds[i].revents & POLLIN) {
        ssize_t length = read(poll_fds[i].fd, buf, sizeof(buf) - 1);
        if (length == -1) {
          handle_error("socket_reader_thread read (poll_fds[%d])", (int)i);
        }

        if (length == 0) {
          printf("ncspot socket closed\n");
          kill(getpid(), SIGUSR1);

          goto close;
        }

        buf[length - 1] = '\0';

        MUTEX(&ctx->mutex, {
                enqueue_socket_message(ctx, ctx->socket_messages, buf);
              });
        pthread_cond_broadcast(&ctx->processor_cond);
      }
    }
  }

close:
  printf("closing socket reader gracefully\n");
  if (close(socket_fd) == -1) {
    handle_error("close (socket)");
  }

  return NULL;
}

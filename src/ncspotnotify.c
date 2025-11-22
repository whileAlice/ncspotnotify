#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "context.h"
#include "debug.h"
#include "error.h"
#include "player_message.h"
#include "mutex.h"
#include "socket_reader.h"
#include "usleep.h"

int
main(void)
{
  Context* ctx = calloc(1, sizeof(Context));
  if (ctx == NULL) {
    perror("calloc (ctx)");
    exit(EXIT_FAILURE);
  }

  if (pipe(ctx->debug_pipe_fds) == -1) {
    handle_error(ctx, "pipe");
    exit(EXIT_FAILURE);
  }

  pthread_mutex_init(&ctx->lock, NULL);

  pthread_t socket_reader_thread_id, debug_thread_id;
  errno = pthread_create(&socket_reader_thread_id, NULL,
                         socket_reader_thread, (void*)ctx);
  if (errno != 0) {
    handle_error(ctx, "pthread_create (reader)");
  }
  errno = pthread_create(&debug_thread_id, NULL,
                         debug_thread, (void*)ctx);
  if (errno != 0) {
    handle_error(ctx, "pthread_create (debug)");
  }

  while (!ctx->should_quit_app) {
    if (ctx->is_socket_message_ready) {
      printf("%s\n", ctx->socket_message);
      PlayerMessage* pm = json_to_player_message(ctx, ctx->socket_message);
      printf("%s - %s\n", pm->playable.artists.data[0], pm->playable.title);
      free_player_message(pm);

      MUTEX(&ctx->lock, { ctx->is_socket_message_ready = false; });
    }

    usleep(100000);
  }

  pthread_join(socket_reader_thread_id, NULL);
  pthread_join(debug_thread_id, NULL);

  errno = pthread_mutex_destroy(&ctx->lock);
  if (errno != 0) {
    handle_error(ctx, "pthread_mutex_destroy");
  }

  bool has_error = ctx->has_error;

  free(ctx);

  if (has_error) {
    exit(EXIT_FAILURE);
  }
}

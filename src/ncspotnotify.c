#include "mutex.h"
#include "processor.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#define _POSIX_C_SOURCE
#include <signal.h>

#include "context.h"
#include "error.h"
#include "notification.h"
#include "socket_reader.h"
#include "socket_messages.h"
#include "notifier.h"
#include "debug.h"

int
main(void)
{
  Context* ctx = calloc(1, sizeof(Context));
  if (ctx == NULL) {
    perror("calloc (ctx)");
    exit(EXIT_FAILURE);
  }

  pthread_mutex_init(&ctx->mutex,          NULL);
  pthread_cond_init (&ctx->processor_cond, NULL);
  pthread_cond_init (&ctx->notifier_cond,  NULL);
  ctx->notifications   = init_notifications  (ctx);
  ctx->socket_messages = init_socket_messages(ctx);

  if (pipe(ctx->debug_pipe) == -1) {
    handle_error(ctx, "debug_pipe");
    exit(EXIT_FAILURE);
  }
  if (pipe(ctx->reader_pipe) == -1) {
    handle_error(ctx, "reader_pipe");
    exit(EXIT_FAILURE);
  }

  sigset_t signal_set;
  int      received_signal;

  sigemptyset    (&signal_set);
  sigaddset      (&signal_set, SIGINT);
  sigaddset      (&signal_set, SIGTERM);
  sigaddset      (&signal_set, SIGUSR1);
  pthread_sigmask(SIG_BLOCK, &signal_set, NULL);

  pthread_t socket_reader_thread_id, processor_thread_id,
            notifier_thread_id, debug_thread_id;

  errno = pthread_create(&socket_reader_thread_id, NULL,
                         socket_reader_thread, (void*)ctx);
  if (errno != 0) {
    handle_error(ctx, "pthread_create (reader)");
  }

  errno = pthread_create(&processor_thread_id, NULL,
                         processor_thread, (void*)ctx);
  if (errno != 0) {
    handle_error(ctx, "pthread_create (processor)");
  }

  errno = pthread_create(&notifier_thread_id, NULL,
                         notifier_thread, (void*)ctx);
  if (errno != 0) {
    handle_error(ctx, "pthread_create (notifier)");
  }

  errno = pthread_create(&debug_thread_id, NULL,
                         debug_thread, (void*)ctx);
  if (errno != 0) {
    handle_error(ctx, "pthread_create (debug)");
  }

  sigwait(&signal_set, &received_signal);

  MUTEX(&ctx->mutex, { ctx->should_quit_app = true; });

  printf("wakie wakie\n");
  // wake up threads
  const uint8_t byte = 0;
  if (write(ctx->debug_pipe[1], &byte, 1) != 1) {
    handle_error(ctx, "write (debug_pipe write end)");
  }
  if (write(ctx->reader_pipe[1], &byte, 1) != 1) {
    handle_error(ctx, "write (reader_pipe write end)");
  }
  pthread_cond_broadcast(&ctx->processor_cond);
  pthread_cond_broadcast(&ctx->notifier_cond);

  pthread_join(debug_thread_id, NULL);
  pthread_join(notifier_thread_id, NULL);
  pthread_join(processor_thread_id, NULL);
  pthread_join(socket_reader_thread_id, NULL);

  if (close(ctx->debug_pipe[0]) == -1) {
    handle_error(ctx, "close (debug_pipe read end)");
  }
  if (close(ctx->debug_pipe[1]) == -1) {
    handle_error(ctx, "close (debug_pipe write end)");
  }
  if (close(ctx->reader_pipe[0]) == -1) {
    handle_error(ctx, "close (reader_pipe read end)");
  }
  if (close(ctx->reader_pipe[1]) == -1) {
    handle_error(ctx, "close (reader_pipe write end)");
  }

  errno = pthread_cond_destroy(&ctx->notifier_cond);
  if (errno != 0) {
    handle_error(ctx, "pthread_reader_cond_destroy");
  }

  errno = pthread_cond_destroy(&ctx->processor_cond);
  if (errno != 0) {
    handle_error(ctx, "pthread_notifier_cond_destroy");
  }

  errno = pthread_mutex_destroy(&ctx->mutex);
  if (errno != 0) {
    handle_error(ctx, "pthread_mutex_destroy");
  }

  bool has_error = ctx->has_error;

  free(ctx);

  if (has_error) {
    exit(EXIT_FAILURE);
  }
}

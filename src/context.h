#pragma once

#include <pthread.h>

typedef struct context {
  pthread_mutex_t lock;

  char message[1024];
  bool is_message_ready;
  bool should_quit_app;
  bool has_error;
  int  debug_pipe_fds[2];
} Context;

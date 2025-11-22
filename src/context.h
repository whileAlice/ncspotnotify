#pragma once

#include <pthread.h>

#include "config.h"

typedef struct context {
  pthread_mutex_t lock;

  char socket_message[SOCKET_BUFFER_SIZE];
  bool is_socket_message_ready;
  bool should_quit_app;
  bool has_error;
  int  debug_pipe_fds[2];
} Context;

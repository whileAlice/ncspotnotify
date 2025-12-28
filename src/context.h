#pragma once

#include <pthread.h>

#include "config.h"

typedef struct socket_messages SocketMessages;
typedef struct notifications   Notifications;

typedef struct context {
   pthread_mutex_t mutex;
   int             reader_pipe[2];
   int             debug_pipe[2];
   pthread_cond_t  processor_cond;
   pthread_cond_t  notifier_cond;
   SocketMessages* socket_messages;
   Notifications*  notifications;
   char            socket_message[MESSAGE_BUFFER_SIZE];
   bool            should_quit_app;
   bool            is_verbose;
} Context;

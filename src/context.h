#pragma once

#include "config.h"
#include "notifications.h"
#include "processes.h"
#include "socket_messages.h"

#include <pthread.h>

typedef struct context {
   pthread_mutex_t mutex;
   int             reader_pipe[2];
   int             debug_pipe[2];
   pthread_cond_t  processor_cond;
   pthread_cond_t  notifier_cond;
   SocketMessages* socket_messages;
   Notifications*  notifications;
   Processes*      processes;
   char            socket_message[MESSAGE_BUFFER_SIZE];
   bool            should_quit_app;
   bool            is_verbose;
} Context;

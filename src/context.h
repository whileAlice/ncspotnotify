#pragma once

#include "notifications.h"
#include "processes.h"
#include "socket_messages.h"

#include <pthread.h>

typedef struct context {
   pthread_mutex_t mutex;
   size_t          ready_thread_count;
   int             reader_pipe[2];
   pthread_cond_t  processor_cond;
   pthread_cond_t  notifier_cond;
   pthread_cond_t  terminator_cond;
   SocketMessages* socket_messages;
   Notifications*  notifications;
   Processes*      processes;
   bool            should_quit_app;
} Context;

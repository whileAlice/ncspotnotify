#pragma once

#include "notifications.h"
#include "processes.h"
#include "socket_messages.h"

#include <pthread.h>

typedef struct context {
   int             poller_pipe[2];
   sigset_t        signal_set;
   pthread_cond_t  processor_cond;
   pthread_cond_t  notifier_cond;
   pthread_cond_t  terminator_cond;
   SocketMessages* socket_messages;
   Notifications*  notifications;
   Processes*      processes;
} Context;

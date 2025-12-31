#include "notifier_thread.h"

#include "config.h"
#include "context.h"
#include "error.h"
#include "log.h"
#include "mutex.h"
#include "notification.h"
#include "notifications.h"
#include "threads.h"

#include <assert.h>
#include <pthread.h>
#include <spawn.h>
#include <stdlib.h>
#include <string.h>

#define ARGC 3

extern char** environ;

void*
notifier_thread (void* args)
{
   Context* ctx = (Context*)args;
   assert (ctx->notifications != NULL);

   IN_LOCK(&g_main_mutex,
   {
      ctx->ready_thread_count += 1;
      pthread_cond_broadcast (&g_main_cond);

      dbg ("waiting for other threads...");
      while ((ctx->ready_thread_count < THREAD_COUNT) && !g_is_failure)
      {
         pthread_cond_wait (&g_main_cond, &g_main_mutex);
      }

      if (g_is_failure)
      {
         dbg ("fatal failure! quitting...");
         goto close;
      }
   });

   pid_t  child_pid;
   char** argv;

   while (!ctx->should_quit_app)
   {
      Notification* notification = NULL;

      IN_LOCK (&ctx->mutex,
      {
         if (ctx->notifications->count == 0)
         {
            dbg ("notifier waiting for notification");
            pthread_cond_wait (&ctx->notifier_cond, &ctx->mutex);
         }
         else
         {
            notification = notifications_dequeue (ctx->notifications);
            if (notification == NULL)
            {
               set_error ("notifications dequeue");
               pthread_mutex_unlock(&ctx->mutex);
               goto close;
            }
         }
      });

      if (notification == NULL)
         continue;

      argv = calloc (ARGC, sizeof (char*));
      if (argv == NULL)
      {
         set_error ("argv calloc");
         goto close;
      }

      argv[0] = strdup (NOTIFICATION_CMD);
      argv[1] = notification_to_string (notification);
      if (argv[1] == NULL)
      {
         set_error ("notification to string");
         goto close;
      }
      argv[2] = NULL;

      notification_free (notification);

      if (posix_spawnp (&child_pid, NOTIFICATION_CMD, NULL, NULL, argv,
                        environ) != 0)
      {
         set_error ("posix spawnp");
         goto close;
      }

      time_t timestamp = time (NULL);

      Process* process = malloc (sizeof (Process));
      if (process == NULL)
      {
         set_error ("process malloc");
         goto close;
      }

      *process =
        (Process){ .pid = child_pid, .timestamp = timestamp, .argv = argv };

      IN_LOCK (&ctx->mutex,
      {
         // takes ownership of process
         if (processes_enqueue (ctx->processes, process) == -1)
         {
            set_error ("processes enqueue");
            pthread_mutex_unlock (&ctx->mutex);
            goto close;
         }
         pthread_cond_broadcast (&ctx->terminator_cond);
      });
   }

close:
   dbg ("closing notifier thread gracefully");

   // TODO: propagate this appropriately
   if (has_thread_error (pthread_self ()))
      IN_LOCK (&ctx->mutex,
         ctx->should_quit_app = true;
      );

   return NULL;
}

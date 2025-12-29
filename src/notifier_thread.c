#include "notifier_thread.h"

#include "config.h"
#include "context.h"
#include "error.h"
#include "log.h"
#include "mutex.h"
#include "notification.h"
#include "notifications.h"

#include <assert.h>
#include <pthread.h>
#include <spawn.h>
#include <stdlib.h>

extern char** environ;

void*
notifier_thread (void* args)
{
   Context* ctx = (Context*)args;
   assert (ctx->notifications != NULL);

   pid_t  child_pid;
   char** argv;

   argv = calloc (2, sizeof (char*));
   if (argv == NULL)
   {
      set_error ("calloc");
      goto close;
   }

   argv[0] = NOTIFICATION_CMD;

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

      argv[1] = notification_to_string (notification);
      if (argv[1] == NULL)
      {
         set_error ("notification to string");
         goto close;
      }

      notification_free (notification);

      if (posix_spawnp (&child_pid, NOTIFICATION_CMD, NULL, NULL, argv,
                        environ) != 0)
      {
         set_error ("posix spawnp");
         goto close;
      }

      time_t   timestamp        = time (NULL);
      Process* process          = process_create (child_pid, timestamp);
      process->notification_ptr = argv[1];

      IN_LOCK (&ctx->mutex,
      {
         if (processes_enqueue (ctx->processes, process) == -1)
         {
            set_error ("processes enqueue");
            pthread_mutex_unlock (&ctx->mutex);
            goto close;
         }
         pthread_cond_broadcast (&ctx->terminator_cond);
      });
   }

   free (argv);

close:
   dbg ("closing notifier thread gracefully");

   // TODO: propagate this appropriately
   if (has_thread_error (pthread_self ()))
      IN_LOCK (&ctx->mutex,
         ctx->should_quit_app = true;
      );

   return NULL;
}

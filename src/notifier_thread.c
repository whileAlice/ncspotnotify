#include "notifier_thread.h"

#include "context.h"
#include "error.h"
#include "log.h"
#include "mutex.h"
#include "notification.h"
#include "notifications.h"

#include <assert.h>
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

   argv    = calloc (2, sizeof (char*));
   argv[0] = NOTIFICATION_CMD;

   do
   {
      IN_LOCK (&ctx->mutex,
      {
         while (ctx->notifications->count > 0)
         {
            Notification* n = notifications_dequeue (ctx->notifications);
                    argv[1] = notification_to_string (n);

            int err =
              posix_spawnp (&child_pid, NOTIFICATION_CMD,
                            NULL, NULL, argv, environ);
            if (err != 0)
            {
               set_error ("posix spawnp");
               goto close;
            }
         };

         dbg ("notifier waiting for notification");
         pthread_cond_wait (&ctx->notifier_cond, &ctx->mutex);
      });
   }
   while (!ctx->should_quit_app);

close:
   dbg ("closing notifier thread gracefully");
   free (argv);

   return NULL;
}

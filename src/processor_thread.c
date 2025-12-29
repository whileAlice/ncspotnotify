#include "processor_thread.h"

#include "context.h"
#include "error.h"
#include "log.h"
#include "mutex.h"
#include "notification.h"
#include "player_message.h"
#include "socket_messages.h"

#include <assert.h>
#include <stdlib.h>

void*
processor_thread (void* args)
{
   Context* ctx = (Context*)args;
   assert (ctx->socket_messages != NULL);

   while (!ctx->should_quit_app)
   {
      char* socket_message_json = NULL;

      IN_LOCK (&ctx->mutex,
      {
         if (ctx->socket_messages->count == 0)
         {
            dbg ("processor waiting for socket message");
            pthread_cond_wait (&ctx->processor_cond, &ctx->mutex);
         }
         else
         {
            socket_message_json = socket_messages_dequeue (ctx->socket_messages);
            if (socket_message_json == NULL)
            {
               set_error ("socket messages dequeue");
               pthread_mutex_unlock (&ctx->mutex);
               goto close;
            }
         }
      });

      // TODO: the second if seems necessary if we want to do away
      // with `while (count > 0)` in order to process messages outside
      // the mutex lock. is there a more elegant solution?
      if (socket_message_json == NULL)
         continue;

      PlayerMessage* pm = json_to_player_message (socket_message_json);
      if (pm == NULL)
      {
         set_error ("json to player message");
         goto close;
      }

      if (pm->playable->id != NULL && pm->mode->state != FINISHED_TRACK)
      {
         Notification* n = player_message_to_notification (pm);
         if (n == NULL)
         {
            set_error ("player message to notification");
            goto close;
         }

         IN_LOCK (&ctx->mutex,
         {
            if (notifications_enqueue (ctx->notifications, n) == -1)
            {
               set_error ("notifications enqueue");
               pthread_mutex_unlock (&ctx->mutex);
               goto close;
            }
            pthread_cond_broadcast (&ctx->notifier_cond);
         });

         notification_free (n);
      }

      player_message_free (pm);
      free (socket_message_json);
   }

close:
   dbg ("closing processor thread gracefully");

   // FIXME: propagate this appropriately
   if (has_thread_error (pthread_self ()))
      IN_LOCK (&ctx->mutex,
         ctx->should_quit_app = true;
      );

   return NULL;
}

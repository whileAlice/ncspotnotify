#include <assert.h>
#include <stdlib.h>

#include "context.h"
#include "error.h"
#include "log.h"
#include "mutex.h"
#include "notification.h"
#include "player_message.h"
#include "processor_thread.h"
#include "socket_messages.h"

void*
processor_thread (void* args)
{
   Context* ctx = (Context*)args;
   assert (ctx->socket_messages != NULL);

   while (!ctx->should_quit_app)
   {
      IN_LOCK (&ctx->mutex,
      {
         dbg ("processor waiting for socket message");
         pthread_cond_wait (&ctx->processor_cond, &ctx->mutex);

         while (ctx->socket_messages->count > 0)
         {
            char* socket_message_json =
               dequeue_socket_message (ctx->socket_messages);

            PlayerMessage* pm = json_to_player_message (socket_message_json);
            if (pm == NULL)
            {
               set_error ("processor_thread json_to_player_message");
               ctx->should_quit_app = true;
               break;
            }

            if (pm->playable == NULL || pm->mode->state == FINISHED_TRACK)
               continue;

            Notification* n = player_message_to_notification (pm);
            enqueue_notification (ctx->notifications, n);
            pthread_cond_broadcast (&ctx->notifier_cond);

            free (socket_message_json);
            free_notification (n);
            free_player_message (pm);
         }
      });
   }

   dbg ("closing processor thread gracefully");

   return NULL;
}

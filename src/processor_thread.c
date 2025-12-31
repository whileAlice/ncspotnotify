#include "processor_thread.h"

#include "context.h"
#include "error.h"
#include "log.h"
#include "mutex.h"
#include "notification.h"
#include "player_message.h"
#include "socket_messages.h"
#include "threads.h"

#include <assert.h>
#include <stdlib.h>

void*
processor_thread (void* args)
{
   Context* ctx = (Context*)args;
   assert (ctx->socket_messages != NULL);

   IN_LOCK(&g_mutex,
   {
      g_ready_thread_count += 1;
      pthread_cond_broadcast (&g_main_cond);

      dbg ("waiting for other threads...");
      while ((g_ready_thread_count < THREAD_COUNT) && !g_should_quit_app)
         pthread_cond_wait (&g_main_cond, &g_mutex);

      if (g_should_quit_app)
      {
         pthread_mutex_unlock (&g_mutex);
         goto close;
      }
   });

   while (true)
   {
      SocketMessage socket_message_json = NULL;

      IN_LOCK (&g_mutex,
      {
         if (g_should_quit_app)
         {
            pthread_mutex_unlock (&g_mutex);
            goto close;
         }

         if (ctx->socket_messages->count == 0)
         {
            dbg ("waiting for socket messages...");
            pthread_cond_wait (&ctx->processor_cond, &g_mutex);
         }
         else
         {
            socket_message_json = socket_messages_dequeue (ctx->socket_messages);
            if (socket_message_json == NULL)
            {
               set_error ("socket messages dequeue");
               pthread_mutex_unlock (&g_mutex);
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

         IN_LOCK (&g_mutex,
         {
            // takes ownership of notification
            if (notifications_enqueue (ctx->notifications, n) == -1)
            {
               set_error ("notifications enqueue");
               pthread_mutex_unlock (&g_mutex);
               goto close;
            }
            pthread_cond_broadcast (&ctx->notifier_cond);
         });
      }

      player_message_free (pm);
      socket_message_free (socket_message_json);
   }

close:
   dbg ("returning...");

   IN_LOCK(&g_mutex,
   {
      g_should_quit_app = true;
      pthread_cond_broadcast (&g_main_cond);
   });

   return NULL;
}

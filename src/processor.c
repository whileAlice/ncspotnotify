#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "processor.h"
#include "context.h"
#include "log.h"
#include "mutex.h"
#include "notification.h"
#include "socket_messages.h"
#include "player_message.h"

void*
processor_thread(void* args)
{
  Context* ctx = (Context*)args;
  assert(ctx->socket_messages != NULL);

  while (!ctx->should_quit_app) {
    MUTEX(&ctx->mutex, {
            dbg("processor waiting for socket message");
            pthread_cond_wait(&ctx->processor_cond, &ctx->mutex);

            while(ctx->socket_messages->count > 0) {
              char*          socket_message_json =
                dequeue_socket_message(ctx, ctx->socket_messages);
              PlayerMessage* pm =
                json_to_player_message(ctx, socket_message_json);

              if (pm->playable == NULL) {
                continue;
              }

              Notification* n = player_message_to_notification(ctx, pm);
              enqueue_notification(ctx, ctx->notifications, n);
              pthread_cond_broadcast(&ctx->notifier_cond);

              free(socket_message_json);
              free_notification(n);
              free_player_message(pm);
            }
          });
  }

  dbg("closing processor thread gracefully");

  return NULL;
}

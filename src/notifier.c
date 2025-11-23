#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>

#include "notifier.h"
#include "notification.h"
#include "context.h"
#include "mutex.h"

void*
notifier_thread(void* args)
{
  Context* ctx = (Context*)args;
  assert(ctx->notifications != NULL);

  while (!ctx->should_quit_app) {
    MUTEX(&ctx->mutex, {
            printf("notifier waiting for notification\n");
            pthread_cond_wait(&ctx->notifier_cond, &ctx->mutex);

            while(ctx->notifications->count > 0) {
              send_notification(ctx, dequeue_notification(ctx, ctx->notifications));
            }
          });
  }

  printf("closing notifier thread gracefully\n");

  return NULL;
}

void
send_notification(Context* ctx, Notification* n)
{
  printf("%s\n", notification_to_string(ctx, n));
}

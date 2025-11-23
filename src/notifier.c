#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>

#include "notifier.h"
#include "log.h"
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
            dbg("notifier waiting for notification");
            pthread_cond_wait(&ctx->notifier_cond, &ctx->mutex);

            while(ctx->notifications->count > 0) {
              send_notification(ctx, dequeue_notification(ctx, ctx->notifications));
            }
          });
  }

  dbg("closing notifier thread gracefully");

  return NULL;
}

void
send_notification(Context* ctx, Notification* n)
{
  msg(notification_to_string(ctx, n));
}

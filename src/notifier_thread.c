#include <assert.h>

#include "notifier_thread.h"
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
              log_notification(dequeue_notification(ctx->notifications));
            }
          });
  }

  dbg("closing notifier thread gracefully");

  return NULL;
}

void
log_notification(Notification* n)
{
  msg(notification_to_string(n));
}

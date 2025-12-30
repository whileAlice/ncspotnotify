#include "notifications.h"

#include "notification.h"
#include "ring_buffer.h"

void
notification_free (Notification* n)
{
   if (n == NULL)
      return;

   free (n->artists);
   free (n->title);
   free (n->album);

   free (n);
}

RING_BUFFER_DEFINE (Notification*, notification, Notifications, notifications,
                    5, 20);

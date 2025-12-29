#include "notifications.h"

#include "error.h"
#include "notification.h"
#include "ring_buffer.h"

#include <string.h>

Notification*
notification_copy (Notification* n)
{
   Notification* c = malloc (sizeof (Notification));
   if (c == NULL)
      set_error ("malloc");

   c->state = n->state;

   c->artists = strdup (n->artists);
   if (c->artists == NULL)
      set_error ("artists strdup");

   c->title = strdup (n->title);
   if (c->title == NULL)
      set_error ("title strdup");

   c->album = strdup (n->album);
   if (c->album == NULL)
      set_error ("album strdup");

   return c;
}

void
notification_free (Notification* n)
{
   free (n->artists);
   free (n->title);
   free (n->album);

   free (n);
}

RING_BUFFER_DEFINE (Notification*, notification, Notifications, notifications,
                    5, 20);

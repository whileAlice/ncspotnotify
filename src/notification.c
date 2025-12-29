#include "notification.h"

#include "config.h"
#include "error.h"
#include "player_message.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNKNOWN_ARTIST "Unknown Artist"

Notification*
player_message_to_notification (PlayerMessage* message)
{
   assert (message->playable != NULL);

   Notification* n = calloc (1, sizeof (Notification));
   if (n == NULL)
   {
      set_error ("calloc");
      return NULL;
   }

   n->state   = message->mode->state;
   n->artists = artists_to_string (message->playable->artists);
   n->title   = strdup (message->playable->title);
   n->album   = strdup (message->playable->album);

   return n;
}

char*
notification_to_string (Notification* n)
{
   char* state = get_state_symbol (n->state);

   int length = snprintf (NULL, 0, NOTIFICATION_FORMAT, state, n->artists,
                          n->title, n->album);
   if (length == -1)
   {
      set_error ("snprintf [1]");
      return NULL;
   }

   char* str = malloc (length * sizeof (char));
   if (str == NULL)
   {
      set_error ("malloc");
      return NULL;
   }

   if (snprintf (str, MESSAGE_BUFFER_SIZE, NOTIFICATION_FORMAT, state,
                 n->artists, n->title, n->album) == -1)
   {
      set_error ("snprintf [2]");
      return NULL;
   }

   return str;
}

char*
get_state_symbol (PlayerState state)
{
   char buf[4];

   switch (state)
   {
   case PAUSED        : strcpy (buf, "\u23f8"); break; // ⏸
   case PLAYING       : strcpy (buf, "\u23f5"); break; // ⏵
   case STOPPED       : strcpy (buf, "\u23f9"); break; // ⏹
   case FINISHED_TRACK: strcpy (buf, "\u2298"); break; // ⊘
   }

   buf[3] = '\0';

   char* str = strdup (buf);
   if (str == NULL)
   {
      set_error ("strdup");
      return NULL;
   }

   return str;
}

char*
artists_to_string (StringArray artists)
{
   char* str;

   if (artists.count < 1)
   {
      str = strdup (UNKNOWN_ARTIST);
      if (str == NULL)
      {
         set_error ("strdup");
         return NULL;
      }
      return str;
   }

   size_t length = (artists.count - 1) * strlen (", ") + 1;
   for (size_t i = 0; i < artists.count; ++i)
      length += strlen (artists.data[i]);

   str = calloc (length, sizeof (char));
   if (str == NULL)
   {
      set_error ("calloc");
      return NULL;
   }

   strcat (str, artists.data[0]);

   for (size_t i = 1; i < artists.count; ++i)
   {
      strcat (str, ", ");
      strcat (str, artists.data[i]);
   }

   return str;
}

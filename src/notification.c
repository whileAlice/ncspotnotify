#include "notification.h"

#include "config.h"
#include "error.h"
#include "log.h"
#include "player_message.h"
#include "utils.h"

#include <assert.h>
#include <curl/curl.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

   n->state     = message->mode->state;
   n->artists   = artists_to_string (message->playable->artists);
   n->title     = strdup (message->playable->title);
   n->album     = strdup (message->playable->album);
   n->cover_url = strdup (message->playable->cover_url);
   n->current_time_s =
     (uint32_t)(time (NULL) - message->mode->secs_since_epoch);
   n->total_time_s = (uint32_t)(round (message->playable->duration / 1000.0));

   return n;
}

void
notification_free (Notification* n)
{
   if (n == NULL)
      return;

   free (n->artists);
   free (n->title);
   free (n->album);
   free (n->cover_url);

   free (n);
}

char*
get_top_notification_string (Notification* n)
{
   char* state = get_state_symbol (n->state);

   int length = snprintf (NULL, 0, NOTIFICATION_TOP, n->title);
   if (length == -1)
   {
      set_error ("snprintf [1]");
      return NULL;
   }

   char* str = malloc (length * sizeof (char) + 1);
   if (str == NULL)
   {
      set_error ("malloc");
      return NULL;
   }

   if (snprintf (str, length + 1, NOTIFICATION_TOP, n->title) == -1)
   {
      set_error ("snprintf [2]");
      return NULL;
   }

   free (state);

   return str;
}

char*
get_bottom_notification_string (Notification* n)
{
   int length = snprintf (NULL, 0, NOTIFICATION_BOTTOM, n->artists, n->album);
   if (length == -1)
   {
      set_error ("snprintf [1]");
      return NULL;
   }

   char* str = malloc (length * sizeof (char) + 1);
   if (str == NULL)
   {
      set_error ("malloc");
      return NULL;
   }

   if (snprintf (str, length + 1, NOTIFICATION_BOTTOM, n->artists, n->album) ==
       -1)
   {
      set_error ("snprintf [2]");
      return NULL;
   }

   return str;
}

char*
get_progress_bar (Notification* n)
{
   uint32_t progress = (uint32_t)round (
     (double)n->current_time_s / (double)n->total_time_s * 100.0);

   int length = snprintf (NULL, 0, PROGRESS_BAR_HINT, progress);
   if (length == -1)
   {
      set_error ("snprintf [1]");
      return NULL;
   }

   char* str = malloc (length * sizeof (char) + 1);
   if (str == NULL)
   {
      set_error ("malloc");
      return NULL;
   }

   if (snprintf (str, length + 1, PROGRESS_BAR_HINT, progress) == -1)
   {
      set_error ("snprintf [2]");
      return NULL;
   }

   dbg (str);

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

char*
download_cover (char* cover_url)
{
   const char* home = getenv ("HOME");
   if (home == NULL)
   {
      msg ("$HOME variable not found, cannot download cover");
      errno = ENOENT;
      return NULL;
   }

   char* filename = get_random_filename (FILENAME_LENGTH);
   int   path_len = snprintf (NULL, 0, COVER_PATH_MASK, home, CACHE_DIR,
                              NCSPOTNOTIFY_DIR, filename, COVER_EXTENSION);
   if (path_len == -1)
   {
      set_error ("snprintf [1]");
      return NULL;
   }

   char* path_str = malloc (path_len * sizeof (char) + 1);
   if (path_str == NULL)
   {
      set_error ("malloc");
      return NULL;
   }

   if (snprintf (path_str, path_len + 1, "%s/%s", home, CACHE_DIR) == -1)
   {
      set_error ("snprintf [2]");
      return NULL;
   }

   mode_t mode = 0755;

   if (mkdir (path_str, mode) != 0 && errno != EEXIST)
   {
      set_error ("mkdir [1]");
      return NULL;
   }
   errno = 0;

   strcat (path_str, "/");
   strcat (path_str, NCSPOTNOTIFY_DIR);

   if (mkdir (path_str, mode) != 0 && errno != EEXIST)
   {
      set_error ("mkdir [2]");
      return NULL;
   }
   errno = 0;

   CURL* curl = curl_easy_init ();
   if (curl == NULL)
   {
      set_error ("curl easy init");
      return NULL;
   }

   if (snprintf (path_str, path_len + 1, COVER_PATH_MASK, home, CACHE_DIR,
                 NCSPOTNOTIFY_DIR, filename, COVER_EXTENSION) == -1)
   {
      set_error ("snprintf [3]");
      return NULL;
   }

   FILE* output_file = fopen (path_str, "wb");
   if (output_file == NULL)
   {
      set_error ("fopen");
      curl_easy_cleanup (curl);
      return NULL;
   }

   curl_easy_setopt (curl, CURLOPT_URL, cover_url);
   curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, fwrite);
   curl_easy_setopt (curl, CURLOPT_WRITEDATA, output_file);

   CURLcode res = curl_easy_perform (curl);
   if (res != CURLE_OK)
   {
      set_error ("curl easy perform: %s", curl_easy_strerror (res));
      return NULL;
   }

   free (filename);
   fclose (output_file);
   curl_easy_cleanup (curl);

   return path_str;
}

#include "notifier_thread.h"

#include "config.h"
#include "context.h"
#include "error.h"
#include "log.h"
#include "mutex.h"
#include "notification.h"
#include "notifications.h"
#include "processes.h"
#include "threads.h"
#include "utils.h"

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <spawn.h>
#include <stdlib.h>
#include <string.h>

extern char** environ;

void*
notifier_thread (void* args)
{
   Context* ctx = (Context*)args;
   assert (ctx->notifications != NULL);

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

   pid_t  child_pid;
   char** argv;

   while (true)
   {
      Notification* notification = NULL;

      IN_LOCK (&g_mutex,
      {
         if (g_should_quit_app)
         {
            pthread_mutex_unlock (&g_mutex);
            goto close;
         }

         if (ctx->notifications->count == 0)
         {
            dbg ("waiting for notifications...");
            pthread_cond_wait (&ctx->notifier_cond, &g_mutex);
         }
         else
         {
            notification = notifications_dequeue (ctx->notifications);
            if (notification == NULL)
            {
               set_error ("notifications dequeue");
               pthread_mutex_unlock(&g_mutex);
               goto close;
            }
         }
      });

      if (notification == NULL)
         continue;

      argv = calloc (ARGC, sizeof (char*));
      if (argv == NULL)
      {
         set_error ("argv calloc");
         goto close;
      }

      // TODO: create a cover cache and reuse covers
      char* cover_path = download_cover (notification->cover_url);
      if (cover_path == NULL)
      {
         if (errno == ENOENT)
            errno = 0;
         else
            msg ("cannot download album cover");

         TODO ("add fallback album cover");
      }

      argv[NOTIFICATION_CMD_INDEX]  = strdup (NOTIFICATION_CMD);
      argv[IMAGE_PATH_SWITCH_INDEX] = strdup (IMAGE_PATH_SWITCH);
      argv[COVER_PATH_INDEX]        = cover_path;
      argv[HINT_SWITCH_INDEX]       = strdup (HINT_SWITCH);
      argv[PROGRESS_BAR_HINT_INDEX] = get_progress_bar (notification);
      argv[TOP_NOTIFICATION_STRING_INDEX] =
        get_top_notification_string (notification);
      if (argv[TOP_NOTIFICATION_STRING_INDEX] == NULL)
      {
         set_error ("notification to string");
         goto close;
      }
      argv[BOTTOM_NOTIFICATION_STRING_INDEX] =
        get_bottom_notification_string (notification);
      if (argv[BOTTOM_NOTIFICATION_STRING_INDEX] == NULL)
      {
         set_error ("notification to string");
         goto close;
      }
      argv[NULL_INDEX] = NULL;

      if (posix_spawnp (&child_pid, NOTIFICATION_CMD, NULL, NULL, argv,
                        environ) != 0)
      {
         set_error ("posix spawnp");
         goto close;
      }

      time_t timestamp = time (NULL);

      Process* process = malloc (sizeof (Process));
      if (process == NULL)
      {
         set_error ("process malloc");
         goto close;
      }

      *process =
        (Process){ .pid = child_pid, .timestamp = timestamp, .argv = argv };

      IN_LOCK (&g_mutex,
      {
         // takes ownership of process/argv
         if (processes_enqueue (ctx->processes, process) == -1)
         {
            set_error ("processes enqueue");
            pthread_mutex_unlock (&g_mutex);
            goto close;
         }
         pthread_cond_broadcast (&ctx->terminator_cond);
      });

      notification_free (notification);
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

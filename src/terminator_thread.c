#include "terminator_thread.h"

#include "config.h"
#include "context.h"
#include "error.h"
#include "log.h"
#include "mutex.h"
#include "processes.h"
#include "signal.h"
#include "threads.h"
#include "time.h"
#include "utils.h"

#include <errno.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

void*
terminator_thread (void* args)
{
   Context* ctx = (Context*)args;
   assert (ctx->processes != NULL);

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
      Process* process = NULL;

      IN_LOCK (&g_mutex,
      {
         if (g_should_quit_app)
         {
            pthread_mutex_unlock (&g_mutex);
            goto close;
         }

         if (ctx->processes->count == 0)
         {
            dbg ("waiting for processes...");
            pthread_cond_wait (&ctx->terminator_cond, &g_mutex);
         }
         else
         {
            process = processes_dequeue (ctx->processes);
            if (process == NULL)
            {
               set_error ("processes dequeue");
               pthread_mutex_unlock (&g_mutex);
               goto close;
            }
         }
      });

      if (process == NULL)
         continue;

      time_t now = time (NULL);
      if (now - process->timestamp < CMD_TIMEOUT)
      {
         unsigned int wait =
           (unsigned int)(CMD_TIMEOUT - (now - process->timestamp));

         dbg ("process with pid %d: waiting %d seconds for timeout...",
              process->pid, wait);

         if (sleep (wait) != 0)
            dbg ("sleep interrupted by signal handler");
      }

      int   status;
      pid_t res = waitpid (process->pid, &status, WNOHANG);
      if (res == -1 && errno == ECHILD)
      {
         dbg ("process with pid %d not found", process->pid);
         errno = 0;
         goto free;
      }

      if (res == process->pid)
      {
         dbg ("process with pid %d terminated on its own", process->pid);
         goto free;
      }

      if (res == 0)
      {
         dbg ("process with pid %d not terminated, sending SIGKILL",
              process->pid);
         kill (process->pid, SIGKILL);
         waitpid (process->pid, &status, 0);
         goto free;
      }

      if (res == -1)
      {
         set_error ("waitpid");
         goto close;
      }

   free:
      if (remove (process->argv[COVER_PATH_INDEX]) == -1)
      {
         set_error ("remove");
         goto close;
      }

      process_free (process);
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

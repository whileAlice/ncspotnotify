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

   IN_LOCK(&g_main_mutex,
   {
      ctx->ready_thread_count += 1;
      pthread_cond_broadcast (&g_main_cond);

      dbg ("waiting for other threads...");
      while ((ctx->ready_thread_count < THREAD_COUNT) && !g_is_failure)
      {
         pthread_cond_wait (&g_main_cond, &g_main_mutex);
      }

      if (g_is_failure)
      {
         dbg ("fatal failure! quitting...");
         goto close;
      }
   });

   while (!ctx->should_quit_app)
   {
      Process* process = NULL;

      IN_LOCK (&ctx->mutex,
      {
         if (ctx->processes->count == 0)
         {
            dbg ("terminator waiting for processes");
            pthread_cond_wait (&ctx->terminator_cond, &ctx->mutex);
         }
         else
         {
            process = processes_dequeue (ctx->processes);
            if (process == NULL)
            {
               set_error ("processes dequeue");
               pthread_mutex_unlock (&ctx->mutex);
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

      UNREACHABLE ();

   free:
      process_free (process);
   }

close:
   dbg ("closing terminator thread gracefully");

   return NULL;
}

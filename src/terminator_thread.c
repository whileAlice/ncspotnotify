#include "terminator_thread.h"

#include "config.h"
#include "context.h"
#include "log.h"
#include "mutex.h"
#include "processes.h"
#include "signal.h"
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

   do
   {
      Process* p = NULL;

      IN_LOCK (&ctx->mutex,
      {
         if (ctx->processes->count == 0)
         {
            dbg ("terminator waiting for processes");
            pthread_cond_wait (&ctx->terminator_cond, &ctx->mutex);
         }
         else
            p = processes_dequeue (ctx->processes);
      });

      if (p == NULL)
         continue;

      time_t now = time (NULL);
      if (now - p->timestamp < CMD_TIMEOUT)
      {
         unsigned int wait = (unsigned int)(CMD_TIMEOUT - (now - p->timestamp));
         dbg ("cmd with pid %d before timeout, waiting %d seconds...", p->pid,
              wait);
         sleep (wait);
      }

      int   status;
      pid_t res = waitpid (p->pid, &status, WNOHANG);
      if (res == -1 && errno == ECHILD)
      {
         dbg ("cmd with pid %d not found", p->pid);
         goto free;
      }

      if (res == p->pid)
      {
         dbg ("cmd with pid %d terminated on its own", p->pid);
         goto free;
      }

      if (res == 0)
      {
         dbg ("cmd with pid %d not terminated, sending SIGKILL", p->pid);
         kill (p->pid, SIGKILL);
         waitpid (p->pid, &status, 0);
         goto free;
      }

      UNREACHABLE();
   free:
      process_free (p);
   }
   while (!ctx->should_quit_app);

   dbg ("closing terminator thread gracefully");

   return NULL;
}

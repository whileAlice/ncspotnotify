#include "processes.h"

#include "error.h"

#include <stdlib.h>

Process*
process_create (pid_t pid, time_t timestamp)
{
   Process* p = malloc (sizeof (Process));
   if (p == NULL)
      set_error ("malloc");
   else
      *p = (Process){ .pid = pid, .timestamp = timestamp };

   return p;
}

Process*
process_copy (Process* p)
{
   Process* copy = malloc (sizeof (Process));
   if (copy == NULL)
      set_error ("malloc");
   else
      *copy = *p;

   return copy;
}

void
process_free (Process* p)
{
   free (p);
}

// NOTE: capacity determines the number of pids held for reap/SIGKILL.
// if max capacity is exceeded, excess pids will remain hanged/defunct.
// 128 should be enough, but a better solution would be to sleep in a
// separate thread or use a non-blocking mechanism for timeouts.
RING_BUFFER_DEFINE (Process*, process, Processes, processes, 32, 128)

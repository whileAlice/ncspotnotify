#include "processes.h"

#include <stdlib.h>

void
process_free (Process* p)
{
   if (p == NULL)
      return;

   if (p->argv == NULL)
      goto free_p;

   size_t i = 0;
   while (p->argv[i] != NULL)
      free (p->argv[i++]);

   free (p->argv);

free_p:
   free (p);
}

// NOTE: capacity determines the number of pids held for reap/SIGKILL.
// if max capacity is exceeded, excess pids will remain hanged/defunct.
// 128 should be enough, but a better solution would be to sleep in a
// separate thread or use a non-blocking mechanism for timeouts.
RING_BUFFER_DEFINE (Process*, process, Processes, processes, 32, 128)

#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "context.h"
#include "debug_thread.h"
#include "error.h"
#include "log.h"

void*
debug_thread (void* args)
{
   Context* ctx = (Context*)args;

   char   buf[MESSAGE_BUFFER_SIZE];
   nfds_t fd_count = 2;

   struct pollfd* poll_fds = calloc (fd_count, sizeof (struct pollfd));
   if (poll_fds == NULL)
      set_error ("debug_thread calloc (poll_fds)");

   poll_fds[0].fd     = STDIN_FILENO;
   poll_fds[1].fd     = ctx->debug_pipe[0];
   poll_fds[0].events = poll_fds[1].events = POLLIN;

   while (true)
   {
      dbg ("debug thread waiting for stuff");

      if (poll (poll_fds, fd_count, -1) == -1)
         set_error ("debug_thread poll");

      if (ctx->should_quit_app)
         break;

      for (nfds_t i = 0; i < fd_count; ++i)
      {
         if (poll_fds[i].revents == 0)
            continue;

         if (poll_fds[i].revents & POLLIN)
         {
            ssize_t length = read (poll_fds[i].fd, buf, sizeof (buf) - 1);
            if (length == -1)
               set_error ("debug_thread read (poll_fds[%d])", (int)i);

            buf[length - 1] = '\0';

            if (strcmp (buf, "quit") == 0)
            {
               kill (getpid (), SIGUSR1);
               goto close;
            }
         }
      }
   }

close:
   dbg ("closing debug thread gracefully");

   return NULL;
}

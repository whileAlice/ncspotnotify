#include "socket_reader_thread.h"

#include "config.h"
#include "context.h"
#include "error.h"
#include "log.h"
#include "mutex.h"
#include "socket_messages.h"
#include "utils.h"

#include <poll.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

enum fds {
   SOCKET_FD = 0,
   PIPE_FD,
   FD_COUNT,
};

void*
socket_reader_thread (void* args)
{
   Context* ctx = (Context*)args;

   struct sockaddr_un socket_address;

   int socket_fd = socket (AF_UNIX, SOCK_STREAM, 0);
   if (socket_fd == -1)
   {
      set_error ("socket");
      goto close;
   }

   socket_address.sun_family = AF_UNIX;
   strncpy (socket_address.sun_path, SOCKET_PATH,
            sizeof (socket_address.sun_path) - 1);

   if (connect (socket_fd, (const struct sockaddr*)&socket_address,
                sizeof (socket_address)) == -1)
   {
      set_error ("connect");
      goto close;
   }

   struct pollfd poll_fds[FD_COUNT];

   poll_fds[SOCKET_FD].fd     = socket_fd;
   poll_fds[PIPE_FD].fd       = ctx->reader_pipe[0];
   poll_fds[SOCKET_FD].events = poll_fds[PIPE_FD].events = POLLIN;

   char buf[MESSAGE_BUFFER_SIZE];
   while (true)
   {
      dbg ("socket reader waiting for socket");

      if (poll (poll_fds, FD_COUNT, -1) == -1)
      {
         set_error ("poll");
         goto close;
      }

      if (ctx->should_quit_app)
         break;

      if (poll_fds[SOCKET_FD].revents & POLLIN)
      {
         ssize_t length = read (poll_fds[SOCKET_FD].fd, buf, sizeof (buf) - 1);
         if (length == -1)
         {
            set_error ("read socket");
            goto close;
         }

         if (length == 0)
         {
            set_error ("ncspot socket closed");
            goto close;
         }

         buf[length - 1] = '\0';

         IN_LOCK (&ctx->mutex,
         {
            if (socket_messages_enqueue (ctx->socket_messages, buf) == -1)
            {
               set_error ("socket messages enqueue");
               pthread_mutex_unlock (&ctx->mutex);
               goto close;
            };
            pthread_cond_broadcast (&ctx->processor_cond);
         });
      }

      if (poll_fds[PIPE_FD].revents & POLLIN)
      {
         ssize_t length = read (poll_fds[SOCKET_FD].fd, buf, sizeof (buf) - 1);
         if (length == -1)
         {
            set_error ("read pipe");
            goto close;
         }

         if (length == 0)
         {
            set_error ("reader pipe closed prematurely");
            goto close;
         }

         UNREACHABLE ();
      }
   }

   if (close (socket_fd) == -1)
      set_error ("socket close");

close:
   dbg ("closing socket reader gracefully");

   // TODO: change this to cond broadcast
   if (has_thread_error (pthread_self ()))
      kill (getpid (), SIGUSR1);

   return NULL;
}

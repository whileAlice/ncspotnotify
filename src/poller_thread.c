#include "poller_thread.h"

#include "config.h"
#include "context.h"
#include "error.h"
#include "log.h"
#include "mutex.h"
#include "socket_messages.h"
#include "threads.h"
#include "utils.h"

#include <poll.h>
#include <signal.h>
#include <string.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

enum fds {
   SOCKET_FD = 0,
   PIPE_FD,
   SIGNAL_FD,
   FD_COUNT,
};

void*
poller_thread (void* args)
{
   Context* ctx = (Context*)args;

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

   int signal_fd = signalfd (-1, &ctx->signal_set, 0);
   if (signal_fd == -1)
   {
      set_error ("signalfd");
      goto close;
   }

   struct pollfd poll_fds[FD_COUNT];

   poll_fds[SOCKET_FD].fd = socket_fd;
   poll_fds[PIPE_FD].fd   = ctx->poller_pipe[0];
   poll_fds[SIGNAL_FD].fd = signal_fd;

   for (size_t i = 0; i < FD_COUNT; ++i)
      poll_fds[i].events = POLLIN;

   char buf[MESSAGE_BUFFER_SIZE];
   while (true)
   {
      dbg ("waiting for socket...");

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

         char* socket_message = strdup (buf);
         IN_LOCK (&ctx->mutex,
         {
            // takes ownership of socket message
            if (socket_messages_enqueue (ctx->socket_messages,
                                         socket_message) == -1)
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
            set_error ("poller pipe closed prematurely");
            goto close;
         }

         UNREACHABLE ();
      }

      if (poll_fds[SIGNAL_FD].revents & POLLIN)
      {
         struct signalfd_siginfo signal_info;

         ssize_t length =
           read (poll_fds[SIGNAL_FD].fd, &signal_info, sizeof (signal_info));

         if (length != sizeof (signal_info))
         {
            set_error ("read signal");
            goto close;
         }

         dbg ("received signal: '%s'", strsignal (signal_info.ssi_signo));

         if (signal_info.ssi_signo == SIGINT ||
             signal_info.ssi_signo == SIGTERM)
            goto close;
      }
   }

   if (close (socket_fd) == -1)
      set_error ("socket close");

close:
   dbg ("closing poller gracefully");

   // TODO: change this to cond broadcast
   if (has_thread_error (pthread_self ()))
      kill (getpid (), SIGUSR1);

   return NULL;
}

#include "args.h"
#include "config.h"
#include "context.h"
#include "error.h"
#include "log.h"
#include "mutex.h"
#include "notifier_thread.h"
#include "processor_thread.h"
#include "socket_messages.h"
#include "socket_reader_thread.h"
#include "terminator_thread.h"

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

pthread_mutex_t g_main_mutex;
pthread_cond_t  g_main_cond;
bool            g_should_exit;

int
main (int argv, char** argc)
{
   set_verbosity (args_to_verbosity (argv, argc));
   msg ("set verbosity to %s", get_verbosity_string ());

   if (access (SOCKET_PATH, F_OK) < 0)
   {
      set_error ("socket not found at %s", SOCKET_PATH);
      goto error_exit;
   }

   Context* ctx = calloc (1, sizeof (Context));
   if (ctx == NULL)
   {
      set_error ("main ctx calloc");
      goto error_exit;
   }

   pthread_mutex_init (&ctx->mutex, NULL);

   pthread_cond_init (&ctx->processor_cond, NULL);
   pthread_cond_init (&ctx->notifier_cond, NULL);
   pthread_cond_init (&ctx->terminator_cond, NULL);

   ctx->notifications   = notifications_init ();
   ctx->socket_messages = socket_messages_init ();
   ctx->processes       = processes_init ();

   if (pipe (ctx->reader_pipe) == -1)
   {
      set_error ("main reader_pipe");
      exit (EXIT_FAILURE);
   }

   sigset_t signal_set;
   int      received_signal;

   sigemptyset (&signal_set);

   sigaddset (&signal_set, SIGINT);
   sigaddset (&signal_set, SIGTERM);
   sigaddset (&signal_set, SIGUSR1);

   pthread_sigmask (SIG_BLOCK, &signal_set, NULL);

   pthread_t socket_reader_thread_id;
   pthread_t processor_thread_id;
   pthread_t notifier_thread_id;
   pthread_t terminator_thread_id;

   errno = pthread_create (&socket_reader_thread_id, NULL, socket_reader_thread,
                           (void*)ctx);
   if (errno != 0)
      set_error ("main pthread_create (reader)");

   errno =
     pthread_create (&processor_thread_id, NULL, processor_thread, (void*)ctx);
   if (errno != 0)
      set_error ("main pthread_create (processor)");

   errno =
     pthread_create (&notifier_thread_id, NULL, notifier_thread, (void*)ctx);
   if (errno != 0)
      set_error ("main pthread_create (notifier)");

   errno = pthread_create (&terminator_thread_id, NULL, terminator_thread,
                           (void*)ctx);
   if (errno != 0)
      set_error ("main pthread_create (terminator)");

   sigwait (&signal_set, &received_signal);

   dbg ("received signal: '%s'", strsignal (received_signal));
   msg ("exiting app");

   IN_LOCK (&ctx->mutex,
      ctx->should_quit_app = true;
      dbg ("main: %d\n", ctx->should_quit_app);
   );

   // wake up threads
   const uint8_t byte = 0;

   if (write (ctx->reader_pipe[1], &byte, 1) != 1)
      set_error ("main write (reader_pipe write end)");

   IN_LOCK (&ctx->mutex,
   {
      pthread_cond_broadcast (&ctx->processor_cond);
      pthread_cond_broadcast (&ctx->notifier_cond);
      pthread_cond_broadcast (&ctx->terminator_cond);
   });

   pthread_join (terminator_thread_id, NULL);
   pthread_join (notifier_thread_id, NULL);
   pthread_join (processor_thread_id, NULL);
   pthread_join (socket_reader_thread_id, NULL);

   if (close (ctx->reader_pipe[0]) == -1)
      set_error ("main close (reader_pipe read end)");
   if (close (ctx->reader_pipe[1]) == -1)
      set_error ("main close (reader_pipe write end)");

   errno = pthread_cond_destroy (&ctx->notifier_cond);
   if (errno != 0)
      set_error ("main pthread_reader_cond_destroy");

   errno = pthread_cond_destroy (&ctx->processor_cond);
   if (errno != 0)
      set_error ("main pthread_notifier_cond_destroy");

   errno = pthread_cond_destroy (&ctx->terminator_cond);
   if (errno != 0)
      set_error ("main pthread_terminator_cond_destroy");

   errno = pthread_mutex_destroy (&ctx->mutex);
   if (errno != 0)
      set_error ("main pthread_mutex_destroy");

   free (ctx);

error_exit:
   if (has_error ())
   {
      print_error (pthread_self ());
      print_error (socket_reader_thread_id);
      return EXIT_FAILURE;
   }

   deinit_errors ();
   return EXIT_SUCCESS;
}

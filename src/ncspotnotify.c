#include "args.h"
#include "config.h"
#include "context.h"
#include "error.h"
#include "log.h"
#include "mutex.h"
#include "socket_messages.h"
#include "threads.h"

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
      set_error ("ctx calloc");
      goto error_exit;
   }

   pthread_mutex_init (&ctx->mutex, NULL);

   pthread_cond_t* conds[COND_COUNT] = {
      [MAIN_COND]       = &g_main_cond,
      [PROCESSOR_COND]  = &ctx->processor_cond,
      [NOTIFIER_COND]   = &ctx->notifier_cond,
      [TERMINATOR_COND] = &ctx->terminator_cond,
   };

   for (size_t i = 0; i < COND_COUNT; ++i)
      pthread_cond_init (conds[i], NULL);

   ctx->notifications = notifications_init ();
   if (ctx->notifications == NULL)
   {
      set_error ("notifications init");
      goto error_exit;
   }
   ctx->socket_messages = socket_messages_init ();
   if (ctx->socket_messages == NULL)
   {
      set_error ("socket messages init");
      goto error_exit;
   }
   ctx->processes = processes_init ();
   if (ctx->processes == NULL)
   {
      set_error ("processes init");
      goto error_exit;
   }

   if (pipe (ctx->reader_pipe) == -1)
   {
      set_error ("reader pipe create");
      goto error_exit;
   }

   // sigset_t signal_set;
   // int      received_signal;

   // sigemptyset (&signal_set);

   // sigaddset (&signal_set, SIGINT);
   // sigaddset (&signal_set, SIGTERM);
   // sigaddset (&signal_set, SIGUSR1);

   // pthread_sigmask (SIG_BLOCK, &signal_set, NULL);

   thread_fn* thread_fns     = get_thread_fns ();
   g_thread_ids[MAIN_THREAD] = pthread_self ();

   for (size_t i = MAIN_THREAD + 1; i < THREAD_COUNT; ++i)
   {
      errno =
        pthread_create (&g_thread_ids[i], NULL, thread_fns[i], (void*)ctx);
      if (errno != 0)
      {
         set_error ("pthread create %s", get_thread_name ((ThreadIdx)i));
         goto error_exit;
      }
   }

   // TODO: replace this with cond wait
   // sigwait (&signal_set, &received_signal);

   // dbg ("received signal: '%s'", strsignal (received_signal));
   IN_LOCK(&g_main_mutex,
   {
      ctx->ready_thread_count += 1;
      g_is_main_waiting        = true;

      dbg ("waiting for other threads...");
      while ((ctx->ready_thread_count < THREAD_COUNT) && !g_is_failure)
      {
         pthread_cond_wait (&g_main_cond, &g_main_mutex);
      }
   });

   if (g_is_failure)
   {
      dbg ("fatal failure! quitting...");
      goto error_exit;
   }

   dbg ("all threads ready!");

   IN_LOCK(&g_main_mutex,
   {
      while (g_is_failure == false)
      {
         pthread_cond_wait (&g_main_cond, &g_main_mutex);
      }
   });

   IN_LOCK (&ctx->mutex,
      ctx->should_quit_app = true;
   );

   // wake up threads
   const uint8_t byte = 0;

   if (write (ctx->reader_pipe[1], &byte, 1) != 1)
   {
      set_error ("reader pipe write");
      goto error_exit;
   }

   IN_LOCK (&ctx->mutex,
   {

      for (size_t i = 0; i < COND_COUNT; ++i)
         pthread_cond_broadcast (conds[i]);
   });

   for (size_t i = MAIN_THREAD + 1; i < THREAD_COUNT; ++i)
   {
      if (pthread_join (g_thread_ids[i], NULL) != 0)
      {
         set_error ("pthread join %s", get_thread_name ((ThreadIdx)i));
         goto error_exit;
      }
   }

   if (close (ctx->reader_pipe[0]) == -1)
   {
      set_error ("reader pipe read end close");
      goto error_exit;
   }
   if (close (ctx->reader_pipe[1]) == -1)
   {
      set_error ("reader pipe write end close");
      goto error_exit;
   }

   for (int i = 0; i < COND_COUNT; ++i)
   {
      errno = pthread_cond_destroy (conds[i]);
      if (errno != 0)
      {
         set_error ("pthread cond destroy %s", get_cond_name ((CondIdx)i));
         goto error_exit;
      }
   }

   errno = pthread_mutex_destroy (&ctx->mutex);
   if (errno != 0)
   {
      set_error ("pthread mutex destroy");
      goto error_exit;
   }

   notifications_free (ctx->notifications);
   socket_messages_free (ctx->socket_messages);
   processes_free (ctx->processes);

   free (ctx);

error_exit:
   if (has_error ())
   {
      for (size_t i = 0; i < THREAD_COUNT; ++i)
      {
         print_error (g_thread_ids[i], get_thread_name ((ThreadIdx)i));
      }

      return EXIT_FAILURE;
   }

   deinit_errors ();
   return EXIT_SUCCESS;
}

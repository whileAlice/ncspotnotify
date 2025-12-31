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
      goto exit;
   }

   Context* ctx = calloc (1, sizeof (Context));
   if (ctx == NULL)
   {
      set_error ("ctx calloc");
      goto exit;
   }

   pthread_mutex_init (&g_mutex, NULL);

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
      goto exit;
   }
   ctx->socket_messages = socket_messages_init ();
   if (ctx->socket_messages == NULL)
   {
      set_error ("socket messages init");
      goto exit;
   }
   ctx->processes = processes_init ();
   if (ctx->processes == NULL)
   {
      set_error ("processes init");
      goto exit;
   }

   if (pipe (ctx->poller_pipe) == -1)
   {
      set_error ("poller pipe create");

      goto exit;
   }

   sigemptyset (&ctx->signal_set);

   sigaddset (&ctx->signal_set, SIGINT);
   sigaddset (&ctx->signal_set, SIGTERM);

   pthread_sigmask (SIG_BLOCK, &ctx->signal_set, NULL);

   thread_fn* thread_fns     = get_thread_fns ();
   g_thread_ids[MAIN_THREAD] = pthread_self ();

   for (size_t i = MAIN_THREAD + 1; i < THREAD_COUNT; ++i)
   {
      errno =
        pthread_create (&g_thread_ids[i], NULL, thread_fns[i], (void*)ctx);
      if (errno != 0)
      {
         set_error ("pthread create %s", get_thread_name ((ThreadIdx)i));
         goto exit;
      }
   }

   IN_LOCK(&g_mutex,
   {
      g_ready_thread_count += 1;
      g_is_main_waiting     = true;
      pthread_cond_broadcast (&g_main_cond);

      dbg ("waiting for the quit flag...");
      while (!g_should_quit_app)
         pthread_cond_wait (&g_main_cond, &g_mutex);
   });

   msg ("exiting...");

   // wake up threads
   const uint8_t byte = 0;

   if (write (ctx->poller_pipe[1], &byte, 1) != 1)
   {
      set_error ("poller pipe write");
      goto exit;
   }

   IN_LOCK (&g_mutex,
      for (size_t i = 0; i < COND_COUNT; ++i)
         pthread_cond_broadcast (conds[i]);
   );

   for (size_t i = MAIN_THREAD + 1; i < THREAD_COUNT; ++i)
   {
      if (pthread_join (g_thread_ids[i], NULL) != 0)
      {
         set_error ("pthread join %s", get_thread_name ((ThreadIdx)i));
         goto exit;
      }
   }

   if (close (ctx->poller_pipe[0]) == -1)
   {
      set_error ("poller pipe read end close");
      goto exit;
   }
   if (close (ctx->poller_pipe[1]) == -1)
   {
      set_error ("poller pipe write end close");
      goto exit;
   }

   for (int i = 0; i < COND_COUNT; ++i)
   {
      errno = pthread_cond_destroy (conds[i]);
      if (errno != 0)
      {
         set_error ("pthread cond destroy %s", get_cond_name ((CondIdx)i));
         goto exit;
      }
   }

   errno = pthread_mutex_destroy (&g_mutex);
   if (errno != 0)
   {
      set_error ("pthread mutex destroy");
      goto exit;
   }

   notifications_free (ctx->notifications);
   socket_messages_free (ctx->socket_messages);
   processes_free (ctx->processes);

   free (ctx);

exit:
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

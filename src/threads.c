#include "threads.h"

#include "notifier_thread.h"
#include "processor_thread.h"
#include "socket_reader_thread.h"
#include "terminator_thread.h"

#include <assert.h>
#include <pthread.h>

pthread_mutex_t g_main_mutex;
pthread_cond_t  g_main_cond;
pthread_t       g_thread_ids[THREAD_COUNT];
bool            g_is_main_waiting;

static thread_fn s_thread_fns[THREAD_COUNT] = {
   [SOCKET_READER_THREAD] = socket_reader_thread,
   [PROCESSOR_THREAD]     = processor_thread,
   [NOTIFIER_THREAD]      = notifier_thread,
   [TERMINATOR_THREAD]    = terminator_thread,
};

static const char* s_thread_names[THREAD_COUNT] = {
   [MAIN_THREAD]          = "main",
   [SOCKET_READER_THREAD] = "socket reader",
   [PROCESSOR_THREAD]     = "processor",
   [NOTIFIER_THREAD]      = "notifier",
   [TERMINATOR_THREAD]    = "terminator",
};

static const char* s_cond_names[COND_COUNT] = {
   [MAIN_COND]       = "main",
   [PROCESSOR_COND]  = "processor",
   [NOTIFIER_COND]   = "notifier",
   [TERMINATOR_COND] = "terminator",
};

thread_fn*
get_thread_fns (void)
{
   return s_thread_fns;
}

const char*
thread_id_to_name (pthread_t thread_id)
{
   for (size_t i = 0; i < THREAD_COUNT; ++i)
      if pthread_equal(g_thread_ids[i], thread_id)
         return get_thread_name((ThreadIdx)i);

   return NULL;
}

const char*
get_thread_name (ThreadIdx pos)
{
   assert (pos >= 0 && pos < THREAD_COUNT);
   return s_thread_names[pos];
}

const char*
get_cond_name (CondIdx pos)
{
   assert (pos >= 0 && pos < COND_COUNT);
   return s_cond_names[pos];
}

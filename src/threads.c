#include "threads.h"

#include "notifier_thread.h"
#include "processor_thread.h"
#include "socket_reader_thread.h"
#include "terminator_thread.h"

#include <assert.h>
#include <pthread.h>

pthread_mutex_t g_main_mutex;
pthread_cond_t  g_main_cond;
bool            g_should_exit;

static thread_fn s_thread_fns[THREAD_COUNT] = {
   [SOCKET_READER_THREAD] = socket_reader_thread,
   [PROCESSOR_THREAD]     = processor_thread,
   [NOTIFIER_THREAD]      = notifier_thread,
   [TERMINATOR_THREAD]    = terminator_thread,
};

static const char* s_thread_names[THREAD_COUNT] = {
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
get_thread_fns ()
{
   return s_thread_fns;
}

const char*
get_thread_name (ThreadId id)
{
   assert (id >= 0 && id < THREAD_COUNT);
   return s_thread_names[id];
}

const char*
get_cond_name (CondId id)
{
   assert (id >= 0 && id < COND_COUNT);
   return s_cond_names[id];
}

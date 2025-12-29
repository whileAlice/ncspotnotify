#include <pthread.h>

#define COND_COUNT   4
#define THREAD_COUNT 4

extern pthread_mutex_t g_main_mutex;
extern pthread_cond_t  g_main_cond;
extern bool            g_should_exit;

typedef void* (*thread_fn) (void* args);

typedef enum thread_id {
   SOCKET_READER_THREAD,
   PROCESSOR_THREAD,
   NOTIFIER_THREAD,
   TERMINATOR_THREAD,
} ThreadId;

typedef enum cond_id {
   MAIN_COND,
   PROCESSOR_COND,
   NOTIFIER_COND,
   TERMINATOR_COND,
} CondId;

// clang-format off
thread_fn*  get_thread_fns  ();
const char* get_thread_name (ThreadId id);
const char* get_cond_name   (CondId   id);

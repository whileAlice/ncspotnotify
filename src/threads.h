#include <pthread.h>

typedef void* (*thread_fn) (void* args);

typedef enum thread_pos {
   MAIN_THREAD,
   SOCKET_READER_THREAD,
   PROCESSOR_THREAD,
   NOTIFIER_THREAD,
   TERMINATOR_THREAD,
   THREAD_COUNT,
} ThreadIdx;

typedef enum cond_pos {
   MAIN_COND,
   PROCESSOR_COND,
   NOTIFIER_COND,
   TERMINATOR_COND,
   COND_COUNT,
} CondIdx;

extern pthread_mutex_t g_main_mutex;
extern pthread_cond_t  g_main_cond;
extern pthread_t       g_thread_ids[THREAD_COUNT];
extern bool            g_is_main_waiting;

// clang-format off
thread_fn*  get_thread_fns    (void);
const char* thread_id_to_name (pthread_t thread_id);
const char* get_thread_name   (ThreadIdx pos);
const char* get_cond_name     (CondIdx   pos);

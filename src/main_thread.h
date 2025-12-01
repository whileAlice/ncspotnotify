#include <pthread.h>

extern pthread_mutex_t g_main_mutex;
extern pthread_cond_t  g_main_cond;
extern bool            g_should_exit;

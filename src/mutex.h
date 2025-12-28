#pragma once

#include <pthread.h>

// clang-format off
#define IN_LOCK(mutex_ptr, statements) \
   do {                                \
      pthread_mutex_lock(mutex_ptr);   \
      statements                       \
      pthread_mutex_unlock(mutex_ptr); \
   } while (0)

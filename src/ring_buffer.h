#include "error.h" // IWYU pragma: keep <- clangd, are you ok? -_-

#include <assert.h>
#include <stdlib.h>

// ring buffer passes ownership of data to dequeue caller

// clang-format off
#define RING_BUFFER_DECLARE(DATA_TYPE, DATA_FUNCTION_PREFIX, BUFFER_TYPE,           \
                            BUFFER_FUNCTION_PREFIX)                                 \
   typedef struct {                                                                 \
      DATA_TYPE* data;                                                              \
      size_t     count;                                                             \
      size_t     head;                                                              \
      size_t     tail;                                                              \
      size_t     capacity;                                                          \
   } BUFFER_TYPE;                                                                   \
                                                                                    \
   BUFFER_TYPE* BUFFER_FUNCTION_PREFIX##_init    (void);                            \
   int          BUFFER_FUNCTION_PREFIX##_enqueue (BUFFER_TYPE* rb, DATA_TYPE item); \
   DATA_TYPE    BUFFER_FUNCTION_PREFIX##_dequeue (BUFFER_TYPE* rb);                 \
   void         BUFFER_FUNCTION_PREFIX##_free    (BUFFER_TYPE* rb);
// clang-format on

#define RING_BUFFER_DEFINE(DATA_TYPE, DATA_FUNCTION_PREFIX, BUFFER_TYPE,    \
                           BUFFER_FUNCTION_PREFIX, INITIAL_BUFFER_CAPACITY, \
                           MAX_BUFFER_CAPACITY)                             \
   BUFFER_TYPE* BUFFER_FUNCTION_PREFIX##_init (void)                        \
   {                                                                        \
      BUFFER_TYPE* rb = malloc (sizeof (BUFFER_TYPE));                      \
      if (rb == NULL)                                                       \
      {                                                                     \
         set_error ("malloc");                                              \
         return NULL;                                                       \
      }                                                                     \
                                                                            \
      rb->data = calloc (INITIAL_BUFFER_CAPACITY, sizeof (DATA_TYPE));      \
      if (rb->data == NULL)                                                 \
      {                                                                     \
         set_error ("data calloc");                                         \
         return NULL;                                                       \
      }                                                                     \
                                                                            \
      rb->count    = 0;                                                     \
      rb->head     = 0;                                                     \
      rb->tail     = 0;                                                     \
      rb->capacity = INITIAL_BUFFER_CAPACITY;                               \
                                                                            \
      return rb;                                                            \
   }                                                                        \
                                                                            \
   int BUFFER_FUNCTION_PREFIX##_enqueue (BUFFER_TYPE* rb, DATA_TYPE item)   \
   {                                                                        \
      if (rb->capacity == rb->count && rb->capacity < MAX_BUFFER_CAPACITY)  \
      {                                                                     \
         size_t new_cap = rb->capacity * 2;                                 \
                                                                            \
         rb = realloc (rb, new_cap * sizeof (BUFFER_TYPE));                 \
         if (rb == NULL)                                                    \
         {                                                                  \
            set_error ("realloc");                                          \
            return -1;                                                      \
         }                                                                  \
                                                                            \
         rb->capacity = new_cap;                                            \
      }                                                                     \
                                                                            \
      rb->data[rb->tail] = item;                                            \
                                                                            \
      rb->tail   = (rb->tail + 1) % rb->capacity;                           \
      rb->count += 1;                                                       \
                                                                            \
      return 0;                                                             \
   }                                                                        \
                                                                            \
   DATA_TYPE BUFFER_FUNCTION_PREFIX##_dequeue (BUFFER_TYPE* rb)             \
   {                                                                        \
      assert (rb->data[rb->head] != NULL);                                  \
                                                                            \
      DATA_TYPE item     = rb->data[rb->head];                              \
      rb->data[rb->head] = NULL;                                            \
                                                                            \
      rb->head   = (rb->head + 1) % rb->capacity;                           \
      rb->count -= 1;                                                       \
                                                                            \
      return item;                                                          \
   }                                                                        \
                                                                            \
   void BUFFER_FUNCTION_PREFIX##_free (BUFFER_TYPE* rb)                     \
   {                                                                        \
      {                                                                     \
         for (size_t i = 0; i < rb->count; i++)                             \
         {                                                                  \
            DATA_FUNCTION_PREFIX##_free (rb->data[i]);                      \
         }                                                                  \
      }                                                                     \
      free (rb->data);                                                      \
      free (rb);                                                            \
   }

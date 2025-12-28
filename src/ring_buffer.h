#include <assert.h>
#include <stdlib.h>

#ifndef INITIAL_BUFFER_CAPACITY
#define INITIAL_BUFFER_CAPACITY 5
#endif

#ifndef MAX_BUFFER_CAPACITY
#define MAX_BUFFER_CAPACITY 20
#endif

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
   void         BUFFER_FUNCTION_PREFIX##_enqueue (BUFFER_TYPE* rb, DATA_TYPE item); \
   DATA_TYPE    BUFFER_FUNCTION_PREFIX##_dequeue (BUFFER_TYPE* rb);                 \
   void         BUFFER_FUNCTION_PREFIX##_free    (BUFFER_TYPE* rb);
// clang-format on

#define RING_BUFFER_DEFINE(DATA_TYPE, DATA_FUNCTION_PREFIX, BUFFER_TYPE,   \
                           BUFFER_FUNCTION_PREFIX)                         \
   BUFFER_TYPE* BUFFER_FUNCTION_PREFIX##_init (void)                       \
   {                                                                       \
      BUFFER_TYPE* rb = malloc (sizeof (BUFFER_TYPE));                     \
      if (rb == NULL)                                                      \
         set_error ("malloc");                                             \
                                                                           \
      rb->data = calloc (INITIAL_BUFFER_CAPACITY, sizeof (DATA_TYPE));     \
      if (rb->data == NULL)                                                \
         set_error ("data calloc");                                        \
                                                                           \
      rb->count    = 0;                                                    \
      rb->head     = 0;                                                    \
      rb->tail     = 0;                                                    \
      rb->capacity = INITIAL_BUFFER_CAPACITY;                              \
                                                                           \
      return rb;                                                           \
   }                                                                       \
                                                                           \
   void BUFFER_FUNCTION_PREFIX##_enqueue (BUFFER_TYPE* rb, DATA_TYPE item) \
   {                                                                       \
      if (rb->capacity == rb->count && rb->capacity < MAX_BUFFER_CAPACITY) \
      {                                                                    \
         size_t new_cap = rb->capacity * sizeof (BUFFER_TYPE) * 2;         \
                                                                           \
         rb = realloc (rb, new_cap);                                       \
         if (rb == NULL)                                                   \
            set_error ("realloc");                                         \
                                                                           \
         rb->capacity = new_cap;                                           \
      }                                                                    \
                                                                           \
      if (rb->data[rb->tail] != NULL)                                      \
         DATA_FUNCTION_PREFIX##_free (rb->data[rb->tail]);                 \
                                                                           \
      rb->data[rb->tail] = DATA_FUNCTION_PREFIX##_copy (item);             \
      if (rb->data[rb->tail] == NULL)                                      \
         set_error ("DATA_FUNCTION_PREFIX##_copy");                        \
                                                                           \
      rb->tail   = (rb->tail + 1) % rb->capacity;                          \
      rb->count += 1;                                                      \
   }                                                                       \
                                                                           \
   DATA_TYPE BUFFER_FUNCTION_PREFIX##_dequeue (BUFFER_TYPE* rb)            \
   {                                                                       \
      assert (rb->data[rb->head] != NULL);                                 \
                                                                           \
      DATA_TYPE item = DATA_FUNCTION_PREFIX##_copy (rb->data[rb->head]);   \
      if (item == NULL)                                                    \
         set_error ("DATA_FUNCTION_PREFIX##_copy");                        \
                                                                           \
      rb->head   = (rb->head + 1) % rb->capacity;                          \
      rb->count -= 1;                                                      \
                                                                           \
      return item;                                                         \
   }                                                                       \
                                                                           \
   void BUFFER_FUNCTION_PREFIX##_free (BUFFER_TYPE* rb)                    \
   {                                                                       \
      {                                                                    \
         for (size_t i = 0; i < rb->count; i++)                            \
         {                                                                 \
            DATA_FUNCTION_PREFIX##_free (rb->data[i]);                     \
         }                                                                 \
      }                                                                    \
      free (rb->data);                                                     \
      rb->data = NULL;                                                     \
   }

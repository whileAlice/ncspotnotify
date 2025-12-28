#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "socket_messages.h"

#define INITIAL_MESSAGES_CAPACITY 5
#define MAX_MESSAGES_CAPACITY     20

// TODO: this could all be unified with notifications
SocketMessages*
init_socket_messages (void)
{
   SocketMessages* ms = malloc (sizeof (SocketMessages));
   if (ms == NULL)
      set_error ("init_socket_messages malloc");

   ms->data = calloc (INITIAL_MESSAGES_CAPACITY, sizeof (char*));
   if (ms->data == NULL)
      set_error ("init_socket_messages calloc");

   ms->count    = 0;
   ms->head     = 0;
   ms->tail     = 0;
   ms->capacity = INITIAL_MESSAGES_CAPACITY;

   return ms;
}

void
enqueue_socket_message (SocketMessages* ms, char* m)
{
   if (ms->capacity == ms->count && ms->capacity < MAX_MESSAGES_CAPACITY)
   {
      size_t new_cap = ms->capacity * sizeof (SocketMessages) * 2;

      ms           = realloc (ms, new_cap);
      ms->capacity = new_cap;
   }

   if (ms->data[ms->tail] != NULL)
      free (ms->data[ms->tail]);

   ms->data[ms->tail] = strdup (m);
   if (ms->data[ms->tail] == NULL)
      set_error ("enqueue_socket_message strdup");

   ms->tail   = (ms->tail + 1) % ms->capacity;
   ms->count += 1;
}

char*
dequeue_socket_message (SocketMessages* ms)
{
   assert (ms->data[ms->head] != NULL);

   char* m = strdup (ms->data[ms->head]);
   if (m == NULL)
      set_error ("dequeue_socket_message strdup");

   ms->head   = (ms->head + 1) % ms->capacity;
   ms->count -= 1;

   return m;
}

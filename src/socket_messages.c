#include "socket_messages.h"

#include "error.h"
#include "ring_buffer.h"

#include <assert.h>
#include <string.h>

SocketMessage
socket_message_copy (SocketMessage sm)
{
   char* copy = strdup (sm);
   if (copy == NULL)
      set_error ("strdup");

   return copy;
}

void
socket_message_free (SocketMessage sm)
{
   free (sm);
}

RING_BUFFER_DEFINE (SocketMessage, socket_message, SocketMessages,
                    socket_messages, 5, 20);

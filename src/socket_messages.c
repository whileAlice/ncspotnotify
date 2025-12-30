#include "socket_messages.h"

#include "ring_buffer.h"

void
socket_message_free (SocketMessage sm)
{
   free (sm);
}

RING_BUFFER_DEFINE (SocketMessage, socket_message, SocketMessages,
                    socket_messages, 5, 20);

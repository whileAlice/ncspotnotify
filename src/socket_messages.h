#pragma once

#include "ring_buffer.h"

typedef char* SocketMessage;

// clang-format off
void          socket_message_free (SocketMessage sm);
SocketMessage socket_message_copy (SocketMessage sm);
SocketMessage socket_message_zero ();

RING_BUFFER_DECLARE (SocketMessage,  socket_message,
                     SocketMessages, socket_messages);

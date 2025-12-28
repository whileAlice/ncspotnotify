#pragma once

#include "ring_buffer.h"

typedef char* SocketMessage;

// clang-format off
SocketMessage socket_message_copy (SocketMessage sm);
void          socket_message_free (SocketMessage sm);

RING_BUFFER_DECLARE (SocketMessage,  socket_message,
                     SocketMessages, socket_messages);

#pragma once

#include <stddef.h>

typedef struct socket_messages {
   char** data;
   size_t count;
   size_t head;
   size_t tail;
   size_t capacity;
} SocketMessages;

// clang-format off
SocketMessages* init_socket_messages   (void);
void            enqueue_socket_message (SocketMessages* ms, char* m);
char*           dequeue_socket_message (SocketMessages* ms);

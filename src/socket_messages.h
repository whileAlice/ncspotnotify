#pragma once

#include <stddef.h>

#include "context.h"

typedef struct socket_messages {
  char** data;
  size_t count;
  size_t head;
  size_t tail;
  size_t capacity;
} SocketMessages;

SocketMessages* init_socket_messages  (Context* ctx);
void            enqueue_socket_message(Context* ctx, SocketMessages* ms, char* m);
char*           dequeue_socket_message(Context* ctx, SocketMessages* ms);

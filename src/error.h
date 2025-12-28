#pragma once

#include <pthread.h>

typedef struct error_node {
   char*              message;
   struct error_node* next;
} ErrorNode;

typedef struct error {
   ErrorNode*    head;
   pthread_t     tid;
   struct error* next;
} Error;

// clang-format off
void deinit_errors   (void);
void deinit_messages (ErrorNode* error_node);
void set_error       (const char* fmt, ...);
void print_error     (pthread_t tid);
bool has_error       (void);

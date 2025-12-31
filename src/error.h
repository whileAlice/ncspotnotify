#pragma once

#include <pthread.h>

extern bool g_is_failure;

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
void deinit_errors    (void);
void deinit_messages  (ErrorNode* error_node);
void set_error        (const char* fmt, ...);
void print_error      (pthread_t tid, const char* thread_name);
bool has_error        (void);
bool has_thread_error (pthread_t tid);

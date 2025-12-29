#pragma once

#include "ring_buffer.h"

#include <spawn.h>
#include <time.h>

typedef struct process {
   pid_t  pid;
   time_t timestamp;
   char*  notification_ptr;
} Process;

// clang-format off
Process* process_zero   ();
Process* process_create (pid_t pid, time_t timestamp);
Process* process_copy   (Process* p);
void     process_free   (Process* p);

RING_BUFFER_DECLARE (Process*,  process,
                     Processes, processes)

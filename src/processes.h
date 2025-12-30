#pragma once

#include "ring_buffer.h"

#include <spawn.h>
#include <time.h>

typedef struct process {
   pid_t  pid;
   time_t timestamp;
   char** argv;
} Process;

void process_free (Process* p);

RING_BUFFER_DECLARE (Process*, process, Processes, processes)

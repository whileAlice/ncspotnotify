#pragma once

#include "ring_buffer.h"

#include <spawn.h>
#include <time.h>

typedef enum argv_index {
   NOTIFICATION_CMD_INDEX,
   IMAGE_PATH_SWITCH_INDEX,
   COVER_PATH_INDEX,
   NOTIFICATION_STRING_INDEX,
   NULL_INDEX,
   ARGC,
} ArgvIndex;

typedef struct process {
   pid_t  pid;
   time_t timestamp;
   char** argv;
} Process;

void process_free (Process* p);

RING_BUFFER_DECLARE (Process*, process, Processes, processes)

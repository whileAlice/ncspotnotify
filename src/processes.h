#pragma once

#include <spawn.h>
#include <stddef.h>
#include <time.h>

typedef struct process {
   pid_t  pid;
   time_t timestamp;
} Process;

typedef struct processes {
   Process* data;
   size_t   count;
   size_t   head;
   size_t   tail;
   size_t   capacity;
} Processes;

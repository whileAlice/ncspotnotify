#include "executor_thread.h"

#include "context.h"
#include "log.h"

#include <spawn.h>
#include <string.h>

extern char** environ;

void*
executor_thread (void* args)
{
   Context* ctx = (Context*)args;

   dbg ("closing executor thread gracefully");

   return NULL;
}

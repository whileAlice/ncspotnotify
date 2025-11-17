#pragma once

#include <pthread.h>

#include "context.h"

void handle_error(Context* ctx, const char* fmt, ...);

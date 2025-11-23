#pragma once

typedef enum verbosity {
  QUIET = 0,
  INFO,
  DEBUG,
} Verbosity;

void        dbg                (const char* fmt, ...);
void        msg                (const char* fmt, ...);
const char* verbosity_to_string(Verbosity v);

#pragma once

typedef enum verbosity {
   QUIET = 0,
   INFO,
   DEBUG,
} Verbosity;

// clang-format off
void        dbg                  (const char* fmt, ...);
void        msg                  (const char* fmt, ...);
void        set_verbosity        (Verbosity verbosity);
const char* get_verbosity_string (void);

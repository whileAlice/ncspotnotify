#include <string.h>

#include "args.h"

bool
get_is_verbose(int argc, char** argv)
{
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "v")  ||
        strcmp(argv[i], "-v") ||
        strcmp(argv[i], "--verbose")) {
      return true;
    }
  }

  return false;
}

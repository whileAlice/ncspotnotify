#include <string.h>

#include "args.h"
#include "log.h"

Verbosity
get_verbosity(int argc, char** argv)
{
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "d") == 0 ||
        strcmp(argv[i], "-d") == 0||
        strcmp(argv[i], "--debug") == 0) {
      return DEBUG;
    }

    if (strcmp(argv[i], "v") == 0  ||
        strcmp(argv[i], "-v") == 0 ||
        strcmp(argv[i], "--verbose") == 0) {
      return INFO;
    }

    if (strcmp(argv[i], "q") == 0  ||
        strcmp(argv[i], "-q") == 0 ||
        strcmp(argv[i], "--quiet") == 0) {
      return QUIET;
    }
  }

  return QUIET;
}

#include <stdio.h>

#if defined(__GNUC__) || defined(__clang__)
#define UNREACHABLE()                                                      \
   do                                                                      \
   {                                                                       \
      fprintf (stderr, "UNREACHABLE: %s:%d in %s()\n", __FILE__, __LINE__, \
               __func__);                                                  \
      fflush (stderr);                                                     \
      abort ();                                                            \
      __builtin_unreachable ();                                            \
   }                                                                       \
   while (0)
#else
#define UNREACHABLE()                                                      \
   do                                                                      \
   {                                                                       \
      fprintf (stderr, "UNREACHABLE: %s:%d in %s()\n", __FILE__, __LINE__, \
               __func__);                                                  \
      fflush (stderr);                                                     \
      abort ();                                                            \
   }                                                                       \
   while (0)
#endif

#include <stdio.h>

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

#define TODO(text)                                                          \
   do                                                                       \
   {                                                                        \
      fputs ("TODO: text: ", stderr);                                      \
      fprintf (stderr, "at %s:%d in %s()\n", __FILE__, __LINE__, __func__); \
      fflush (stderr);                                                      \
      abort ();                                                             \
   }                                                                        \
   while (0)

char* get_random_filename (size_t length);

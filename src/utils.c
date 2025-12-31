#include "utils.h"

#include "error.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

char*
get_random_filename (size_t length)
{
   srand ((unsigned int)time (NULL));

   const char* charset = "abcdefghijklmnopqrstuvwxyz0123456789";

   char* filename = malloc (sizeof (char) * length + 1);
   if (filename == NULL)
   {
      set_error ("malloc");
      return NULL;
   }

   for (size_t i = 0; i < length; ++i)
   {
      size_t key  = rand () % strlen (charset);
      filename[i] = charset[key];
   }
   filename[length] = '\0';

   return filename;
}

#include "error.h"

#include "main_thread.h"

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Error* s_errors_head = NULL;

void
deinit_errors (void)
{
   Error* current = s_errors_head;
   Error* next;

   while (current != NULL)
   {
      next = current->next;
      deinit_messages (current->head);
      free (current);
      current = next;
   }
}

void
deinit_messages (ErrorNode* error_node)
{
   ErrorNode* current = error_node;
   ErrorNode* next;

   while (current != NULL)
   {
      next = current->next;
      free (current->message);
      free (current);
      current = next;
   }
}

void
set_error (const char* fmt, ...)
{
   pthread_mutex_lock (&g_main_mutex);

   va_list args1, args2;

   va_start (args1, fmt);
   va_copy (args2, args1);

   int length = vsnprintf (NULL, 0, fmt, args1);

   if (length < 0)
   {
      perror ("set_error NULL vsnprintf");
      goto set_error_exit;
   }

   va_end (args1);

   char* error_str = calloc (length + 1, sizeof (char));

   if (error_str == NULL)
   {
      perror ("set_error error_str calloc");
      goto set_error_exit;
   }

   if (vsnprintf (error_str, length + 1, fmt, args2) < 0)
   {
      perror ("set_error error_str vsnprintf");
      goto set_error_exit;
   }

   va_end (args2);

   assert (strlen (error_str) == (size_t)length);

   Error*    current_error = s_errors_head;
   Error*    thread_error  = NULL;
   pthread_t tid           = pthread_self ();

   while (current_error != NULL)
   {
      if (current_error->tid == tid)
      {
         thread_error = current_error;
         break;
      }
      current_error = current_error->next;
   }

   if (current_error == NULL)
   {
      thread_error = calloc (1, sizeof (Error));
      if (thread_error == NULL)
      {
         perror ("set_error thread_error calloc");
         goto set_error_exit;
      }

      thread_error->tid  = tid;
      thread_error->next = s_errors_head;
      s_errors_head      = thread_error;
   }

   ErrorNode* new_error_head = malloc (sizeof (ErrorNode));
   if (new_error_head == NULL)
   {
      perror ("set_error new_error_head malloc");
      goto set_error_exit;
   }

   new_error_head->message = error_str;
   new_error_head->next    = thread_error->head;
   thread_error->head      = new_error_head;

set_error_exit:
   g_should_exit = true;
   pthread_cond_signal (&g_main_cond);

   pthread_mutex_unlock (&g_main_mutex);
}

void
print_error (pthread_t tid)
{
   Error* current = s_errors_head;
   Error* found   = NULL;

   while (current != NULL)
   {
      if (current->tid == tid)
      {
         found = current;
         break;
      }
      current = current->next;
   }

   if (found == NULL)
   {
      return;
   }

   ErrorNode* en = found->head;
   assert (en->message != NULL);

   fputs ("ERROR: ", stderr);
   fputs (en->message, stderr);

   while (en->next != NULL)
   {
      fputs (": ", stderr);

      en = en->next;
      fputs (en->message, stderr);
   }

   if (errno != 0)
   {
      fputs (" (", stderr);
      fputs (strerror (errno), stderr);
      fputs (")", stderr);
   }

   fputc ('\n', stderr);
}

bool
has_error (void)
{
   return s_errors_head != NULL;
}

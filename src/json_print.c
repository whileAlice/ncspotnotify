#include "json_print.h"

#include "json_parse.h"

#include <stdio.h>

const char* JSON_TYPE_STRINGS[] = {
   "String", "Number", "Boolean", "Null", "Object", "Array", "Unknown",
};

void
json_print (JsonNode* root_node)
{
   json_print_recurse (root_node, 0, 2);
}

// TODO: this should build a string first
void
json_print_recurse (JsonNode* node, size_t iteration, size_t indent)
{
   switch (node->type)
   {
   case STRING: printf ("\"%s\"", node->data.string); break;
   case NUMBER: printf ("%f", node->data.number); break;
   case BOOLEAN:
      if (node->data.boolean)
         printf ("true");
      else
         printf ("false");

      break;
   case JSON_NULL: printf ("null"); break;
   case OBJECT:
      printf ("{\n");
      for (size_t i = 0; i < node->children_count; ++i)
      {
         print_indent (iteration + 1, indent);
         printf ("\"%s\": ", node->data.object_children[i].key);
         json_print_recurse (node->data.object_children[i].value, iteration + 1,
                             indent);

         if (i < node->children_count - 1)
            printf (",");

         printf ("\n");
      }

      print_indent (iteration, indent);
      printf ("}");

      break;
   case ARRAY:
      printf ("[\n");
      for (size_t i = 0; i < node->children_count; ++i)
      {
         print_indent (iteration + 1, indent);
         json_print_recurse (node->data.array_children[i], iteration + 1,
                             indent);

         if (i < node->children_count - 1)
            printf (",");

         printf ("\n");
      }

      print_indent (iteration, indent);
      printf ("]");

      break;
   default: printf ("unknown JSON type");
   }

   if (iteration == 0)
      printf ("\n");
}

void
print_indent (size_t iteration, size_t indent)
{
   for (size_t i = 0; i < iteration * indent; ++i)
      printf (" ");
}

const char*
json_type_to_string (JsonType type)
{
   if (type >= 0 && type <= UNKNOWN)
      return JSON_TYPE_STRINGS[type];

   return "UnknownJsonType";
}

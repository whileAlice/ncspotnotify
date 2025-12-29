#pragma once

#include <stddef.h>

typedef enum json_type {
   STRING = 0,
   NUMBER,
   BOOLEAN,
   JSON_NULL,
   OBJECT,
   ARRAY,
   UNKNOWN,
} JsonType;

typedef struct json_node JsonNode;

typedef struct json_member {
   char*     key;
   JsonNode* value;
} JsonMember;

struct json_node {
   JsonType type;
   size_t   children_count;
   union {
      bool   boolean;
      double number;
      char*  string;

      JsonNode**  array_children;
      JsonMember* object_children;
   } data;
};

// clang-format off
JsonNode* json_parse         (char* json_string);
JsonNode* json_parse_value   (char** json_pos);
JsonNode* json_parse_string  (char** json_pos);
JsonNode* json_parse_number  (char** json_pos);
JsonNode* json_parse_boolean (char** json_pos);
JsonNode* json_parse_null    (char** json_pos);
JsonNode* json_parse_object  (char** json_pos);
JsonNode* json_parse_array   (char** json_pos);
char*     parse_string       (char** str);
JsonType  char_to_json_type  (char ch);
void      skip_whitespace    (char** str);
void      json_node_free     (JsonNode* node);

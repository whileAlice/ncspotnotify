#pragma once

#include "context.h"

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
    bool boolean;
    double number;
    char* string;

    JsonNode** array_children;
    JsonMember* object_children;
  } data;
};

JsonNode* json_parse         (Context* ctx, char* json_string);
JsonNode* json_parse_value   (Context* ctx, char** json_pos);
JsonNode* json_parse_string  (Context* ctx, char** json_pos);
JsonNode* json_parse_number  (Context* ctx, char** json_pos);
JsonNode* json_parse_boolean (Context* ctx, char** json_pos);
JsonNode* json_parse_null    (Context* ctx, char** json_pos);
JsonNode* json_parse_object  (Context* ctx, char** json_pos);
JsonNode* json_parse_array   (Context* ctx, char** json_pos);

char*       parse_string       (Context* ctx, char** str);
JsonType    char_to_json_type  (Context* ctx, char ch);
const char* json_type_to_string(JsonType type);
void        skip_whitespace    (char** str);

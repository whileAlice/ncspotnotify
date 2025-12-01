#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "json_parse.h"
#include "config.h"
#include "error.h"

// TODO: free after failure, so that parsing errors are not fatal
JsonNode*
json_parse(char* json_string)
{
  return json_parse_value(&json_string);
}

JsonNode*
json_parse_value(char** json_pos)
{
  assert(**json_pos != '\0');

  skip_whitespace(json_pos);

  JsonNode* node = NULL;
  JsonType  type = char_to_json_type(**json_pos);

  if        (type == STRING) {
    node = json_parse_string(json_pos);
  } else if (type == NUMBER) {
    node = json_parse_number(json_pos);
  } else if (type == BOOLEAN) {
    node = json_parse_boolean(json_pos);
  } else if (type == JSON_NULL) {
    node = json_parse_null(json_pos);
  } else if (type == OBJECT) {
    node = json_parse_object(json_pos);
  } else if (type == ARRAY) {
    node = json_parse_array(json_pos);
  } else {
    return NULL;
  }

  if (node == NULL) {
    return NULL;
  }

  skip_whitespace(json_pos);

  return node;
}

JsonNode*
json_parse_string(char** json_pos)
{
  JsonNode* node = calloc(1, sizeof(JsonNode));
  if (node == NULL) {
    set_error("json_parse_string node calloc");

    return NULL;
  }

  node->type        = STRING;
  node->data.string = parse_string(json_pos);

  return node;
}

JsonNode*
json_parse_number(char** json_pos)
{
  JsonNode* node = calloc(1, sizeof(JsonNode));
  if (node == NULL) {
    set_error("json_parse_number node calloc");

    return NULL;
  }

  char buf[MESSAGE_BUFFER_SIZE];

  size_t i = 0;
  while (isdigit(**json_pos) || **json_pos == '.' || **json_pos == '-') {
    assert(**json_pos != '\0');
    assert(i < MESSAGE_BUFFER_SIZE);

    buf[i++] = *(*json_pos)++;
  }

  buf[i] = '\0';

  node->type        = NUMBER;
  node->data.number = atof(buf);

  return node;
}

JsonNode*
json_parse_boolean(char** json_pos)
{
  JsonNode* node = calloc(1, sizeof(JsonNode));
  if (node == NULL) {
    set_error("json_parse_boolean node calloc");

    return NULL;
  }

  bool boolean;

  if (**json_pos == 't') {
    boolean = true;
    *json_pos += strlen("true");
  } else {
    boolean = false;
    *json_pos += strlen("false");
  }

  node->type         = BOOLEAN;
  node->data.boolean = boolean;

  return node;
}

JsonNode*
json_parse_null(char** json_pos)
{
  JsonNode* node = calloc(1, sizeof(JsonNode));
  if (node == NULL) {
    set_error("json_parse_null node calloc");

    return NULL;
  }

  *json_pos += strlen("null");

  node->type = JSON_NULL;

  return node;
}

JsonNode*
json_parse_object(char** json_pos)
{
  JsonNode* node = calloc(1, sizeof(JsonNode));
  if (node == NULL) {
    set_error("json_parse_object node calloc");

    return NULL;
  }

  *json_pos += 1;
  skip_whitespace(json_pos);

  if (**json_pos == '}') {
    *json_pos += 1;

    return node;
  }

  JsonMember* children = malloc(sizeof(JsonMember));
  if (children == NULL) {
    set_error("json_parse_object children malloc");

    return NULL;
  }

  size_t i = 0;
  while (**json_pos != '}') {
    children[i].key = parse_string(json_pos);
    if (children[i].key == NULL) {
      set_error("json_parse_object parse_string");

      return NULL;
    }

    if(**json_pos != ':') {
      set_error("json_parse_object: expected ':', found '%c'",
                   **json_pos);
      return NULL;
    }

    *json_pos += 1;
    skip_whitespace(json_pos);

    children[i].value = json_parse_value(json_pos);
    if (children[i].value == NULL) {
      set_error("json_parse_object json_parse_value");

      return NULL;
    }

    if (**json_pos == ',') {
      i += 1;
      *json_pos += 1;

      children = realloc(children, sizeof(JsonMember) * (i + 1));
      if (children == NULL) {
        set_error("json_parse_object children realloc");

        return NULL;
      }
    }

    skip_whitespace(json_pos);
  }

  skip_whitespace(json_pos);
  if (**json_pos == '}') {
    *json_pos += 1;
  } else {
    set_error("json_parse_object: expected '}', got '%c'",
                 **json_pos);
    return NULL;
  }

  node->type                 = OBJECT;
  node->data.object_children = children;
  node->children_count       = i + 1;

  return node;
}

JsonNode*
json_parse_array(char** json_pos)
{
  JsonNode* node = calloc(1, sizeof(JsonNode));
  if (node == NULL) {
    set_error("json_parse_array node calloc");

    return NULL;
  }

  *json_pos += 1;
  skip_whitespace(json_pos);

  if (**json_pos == ']') {
    *json_pos += 1;

    return node;
  }

  JsonNode** children = malloc(sizeof(JsonNode*));
  if (children == NULL) {
    set_error("json_parse_array children malloc");

    return NULL;
  }

  size_t i = 0;
  while (**json_pos != ']') {
    children[i] = json_parse_value(json_pos);
    if (children[i] == NULL) {
      set_error("json_parse_array json_parse_value");

      return NULL;
    }

    if (**json_pos != ',') break;

    i += 1;
    *json_pos += 1;
    skip_whitespace(json_pos);

    children = realloc(children, sizeof(JsonNode*) * (i + 1));
    if (children == NULL) {
      set_error("json_parse_array children realloc");

      return NULL;
    }
  }

  skip_whitespace(json_pos);
  if (**json_pos == ']') {
    *json_pos += 1;
  } else {
    set_error("json_parse_array: expected ']', got '%c'",
                 **json_pos);
    return NULL;
  }

  node->type                = ARRAY;
  node->data.array_children = children;
  node->children_count      = i + 1;

  return node;
}

char*
parse_string(char** str)
{
  char buf[MESSAGE_BUFFER_SIZE];

  size_t i = 0;
  while (true) {
    assert(i < MESSAGE_BUFFER_SIZE);

    *str += 1;
    assert(**str != '\0');

    if (**str == '"') {
      buf[i] = '\0';

      break;
    }

    if (**str == '\\') {
      buf[i] = *str[1];
      *str += 1;
      i += 1;

      continue;
    }

    buf[i] = **str;
    i += 1;
  }

  *str += 1;

  char* string = strndup(buf, MESSAGE_BUFFER_SIZE - 1);
  if (string == NULL) {
    set_error("parse_string strndup");

    return NULL;
  }

  return string;
}

JsonType
char_to_json_type(char ch)
{
  if        (ch == '"') {
    return STRING;
  } else if ((ch >= '0' && ch <= '9') || ch == '-') {
    return NUMBER;
  } else if (ch == 't' || ch == 'f') {
    return BOOLEAN;
  } else if (ch == 'n') {
    return JSON_NULL;
  } else if (ch == '{') {
    return OBJECT;
  } else if (ch == '[') {
    return ARRAY;
  } else {
    set_error("char_to_json_type: unknown json type (char: '%c')", ch);
  }

  return UNKNOWN;
}

void
skip_whitespace(char** str)
{
  while (isspace(**str)) *str += 1;
}

void
free_json_node(JsonNode* node)
{
  switch (node->type) {
  case STRING:
    free(node->data.string);
    break;
  case OBJECT:
    for (size_t i = 0; i < node->children_count; ++i) {
      free(node->data.object_children[i].key);
      free_json_node(node->data.object_children[i].value);
    }

    free(node->data.object_children);

    break;
  case ARRAY:
    for (size_t i = 0; i < node->children_count; ++i) {
      free_json_node(node->data.array_children[i]);
    }

    free(node->data.array_children);

    break;
  case NUMBER:
  case BOOLEAN:
  case JSON_NULL:
  default:
  }
}

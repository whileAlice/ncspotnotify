#pragma once

#include "json_parse.h"

void        json_print         (JsonNode* root_node);
void        json_print_recurse (JsonNode* node, size_t iteration, size_t indent);
void        print_indent       (size_t iteration, size_t indent);
const char* json_type_to_string(JsonType type);

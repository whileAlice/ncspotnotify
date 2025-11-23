#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "player_message.h"
#include "context.h"
#include "error.h"
#include "json_parse.h"
#include "json_print.h"

PlayerMessage*
json_to_player_message(char* json_string)
{
  PlayerMessage* pm = calloc(1, sizeof(PlayerMessage));

  JsonNode* json = json_parse(json_string);
  assert(json->type == OBJECT);

  auto* json_mode = get_child(json, 0, "mode");
  pm->mode = parse_json_mode(json_mode);

  auto* json_playable = get_child(json, 1, "playable");
  if (json_playable->value->type == JSON_NULL) {
    pm->playable = NULL;
  } else {
    pm->playable = parse_json_playable(json_playable);
  }

  free_json_node(json);

  return pm;
}

PlayerMode*
parse_json_mode(JsonMember* json_mode)
{
  PlayerMode* m = calloc(1, sizeof(PlayerMode));
  if (m == NULL) {
    handle_error("parse_json_mode calloc");
  }

  // string mode
  if (json_mode->value->type == STRING) {
    if (strcmp(json_mode->value->data.string, "Stopped") == 0) {
      m->state = STOPPED;
    } else {
      handle_error("json_to_player_message: unknown mode state: \"%s\"\n",
                   json_mode->value->data.string);
    }

    return m;
  }

  // k-v pair mode
  auto* state = get_child(json_mode->value, 0, NULL);

  if        (strcmp(state->key, "Paused") == 0) {
    m->state = PAUSED;
    m->secs  = get_child_uint32_t(state, 0, "secs");
    m->nanos = get_child_uint32_t(state, 1, "nanos");
  } else if (strcmp(state->key, "Playing") == 0) {
    m->state             = PLAYING;
    m->secs_since_epoch  = get_child_uint32_t(state, 0,
                                              "secs_since_epoch");
    m->nanos_since_epoch = get_child_uint32_t(state, 1,
                                              "nanos_since_epoch");
  } else {
    handle_error("json_to_player_message: unknown mode state: \"%s\"\n",
                 state->key);
  }

  return m;
}

PlayerPlayable*
parse_json_playable(JsonMember* json_playable)
{
  PlayerPlayable* p = calloc(1, sizeof(PlayerPlayable));
  if (p == NULL) {
    handle_error("parse_json_playable calloc");
  }

  auto* jp = json_playable;

  p->type          = get_child_playable_type(jp, 0,  "type");
  p->id            = get_child_string       (jp, 1,  "id");
  p->uri           = get_child_string       (jp, 2,  "uri");
  p->title         = get_child_string       (jp, 3,  "title");
  p->track_number  = get_child_uint8_t      (jp, 4,  "track_number");
  p->disc_number   = get_child_uint8_t      (jp, 5,  "disc_number");
  p->duration      = get_child_uint32_t     (jp, 6,  "duration");
  p->artists       = get_child_string_array (jp, 7,  "artists");
  p->artist_ids    = get_child_string_array (jp, 8,  "artist_ids");
  p->album         = get_child_string       (jp, 9,  "album");
  p->album_id      = get_child_string       (jp, 10, "album_id");
  p->album_artists = get_child_string_array (jp, 11, "album_artists");
  p->cover_url     = get_child_string       (jp, 12, "cover_url");
  p->url           = get_child_string       (jp, 13, "url");
  p->added_at      = get_child_string       (jp, 14, "added_at");
  p->list_index    = get_child_uint8_t      (jp, 15, "list_index");
  p->is_local      = get_child_boolean      (jp, 16, "is_local");
  p->is_playable   = get_child_boolean      (jp, 17, "is_playable");

  return p;
}

void
free_player_message(PlayerMessage* pm)
{
  auto* p = pm->playable;

  free             (pm->mode);
  free             (p->id);
  free             (p->uri);
  free             (p->title);
  free_string_array(p->artists);
  free_string_array(p->artist_ids);
  free             (p->album);
  free             (p->album_id);
  free_string_array(p->album_artists);
  free             (p->cover_url);
  free             (p->url);
  free             (p->added_at);
  free             (p);
  free             (pm);
}

JsonMember*
get_child(JsonNode* parent, size_t index, const char* name)
{
  assert(parent->type == OBJECT);
  assert(parent->children_count >= index + 1);

  JsonMember* m = &parent->data.object_children[index];
  if (name != NULL) {
    assert(strcmp(m->key, name) == 0);
  }

  return m;
}

char*
get_child_string(JsonMember* parent,
                 size_t index, const char* name)
{
  JsonMember* source_member = get_child(parent->value, index, name);
  assert(source_member->value->type == STRING);

  char* string = source_member->value->data.string;
  source_member->value->data.string = NULL;

  return string;
}

double
get_child_number(JsonMember* parent,
                 size_t index, const char* name)
{
  JsonMember* source_member = get_child(parent->value, index, name);
  assert(source_member->value->type == NUMBER);

  return source_member->value->data.number;
}

uint32_t
get_child_uint32_t(JsonMember* parent,
                   size_t index, const char* name)
{
  return (uint32_t)get_child_number(parent, index, name);
}

uint8_t
get_child_uint8_t(JsonMember* parent,
                  size_t index, const char* name)
{
  return (uint8_t)get_child_number(parent, index, name);
}

bool
get_child_boolean(JsonMember* parent,
                  size_t index, const char* name)
{
  JsonMember* source_member = get_child(parent->value, index, name);
  assert(source_member->value->type == BOOLEAN);

  return source_member->value->data.boolean;
}

StringArray
get_child_string_array(JsonMember* parent,
                       size_t index, const char* name)
{
  JsonMember* source_member = get_child(parent->value, index, name);
  assert(strcmp(source_member->key, name) == 0);
  assert(source_member->value->type == ARRAY);

  size_t count = source_member->value->children_count;

  if (count == 0) {
    return (StringArray){ .data = NULL, .count = 0 };
  }

  StringArray array = {
    .data  = calloc(1, sizeof(char*) * count),
    .count = 0,
  };

  for (size_t i = 0; i < count; ++i) {
    auto* source_child = source_member->value->data.array_children[i];
    assert(source_child->type == STRING);

    array.data[i] = source_child->data.string;
    array.count  += 1;

    source_child->data.string = NULL;
  }

  return array;
}

PlayableType
get_child_playable_type(JsonMember* parent,
                        size_t index, const char* name)
{
  JsonMember* source_member = get_child(parent->value, index, name);
  assert(source_member->value->type == STRING);

  char* string = source_member->value->data.string;
  if (strcmp(string, "Track") == 0 ){
    return TRACK;
  }

  handle_error("get_child_playable_type: unknown type: %s\n", string);

  return UNKNOWN_PLAYABLE_TYPE;
}

void
free_string_array(StringArray s)
{
  for (size_t i = 0; i < s.count; ++i) {
    free(s.data[i]);
  }
}

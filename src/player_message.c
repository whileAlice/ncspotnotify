#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#define _POSIX_C_SOURCE 200809L
#include <string.h>

#include "player_message.h"
#include "context.h"
#include "error.h"
#include "json_parse.h"

PlayerMessage*
json_to_player_message(Context* ctx, char* json_string)
{
  PlayerMessage* pm = calloc(1, sizeof(PlayerMessage));

  JsonNode* json = json_parse(ctx, json_string);
  assert(json->type == OBJECT);

  auto* json_mode = get_child(json, 0, "mode");
  parse_player_mode(ctx, &pm->mode, json_mode);

  auto* json_playable = get_child(json, 1, "playable");
  parse_player_playable(ctx, &pm->playable, json_playable);

  free_json_node(json);

  return pm;
}

void
parse_player_mode(Context* ctx, PlayerMode* m, JsonMember* json_mode)
{
  auto* state = get_child(json_mode->value, 0, NULL);

  if        (strcmp(state->key, "Paused") == 0) {
    m->state = PAUSED;
    m->secs  = get_child_uint32_t(ctx, state, 0, "secs");
    m->nanos = get_child_uint32_t(ctx, state, 1, "nanos");
  } else if (strcmp(state->key, "Stopped") == 0) {
    m->state = STOPPED;
  } else if (strcmp(state->key, "Playing") == 0) {
    m->state = PLAYING;
    m->secs_since_epoch  = get_child_uint32_t(ctx, state, 0, "secs_since_epoch");
    m->nanos_since_epoch = get_child_uint32_t(ctx, state, 1, "nanos_since_epoch");
  } else {
    handle_error(ctx, "json_to_player_message: unknown mode state: %s\n", state);
  }
}

void
parse_player_playable(Context* ctx, PlayerPlayable* p, JsonMember* json_playable)
{
  auto* jp = json_playable;

  p->type          = get_child_playable_type(ctx, jp, 0,  "type");
  p->id            = get_child_string       (ctx, jp, 1,  "id");
  p->uri           = get_child_string       (ctx, jp, 2,  "uri");
  p->title         = get_child_string       (ctx, jp, 3,  "title");
  p->track_number  = get_child_uint8_t      (ctx, jp, 4,  "track_number");
  p->disc_number   = get_child_uint8_t      (ctx, jp, 5,  "disc_number");
  p->duration      = get_child_uint32_t     (ctx, jp, 6,  "duration");
  p->artists       = get_child_string_array (ctx, jp, 7,  "artists");
  p->artist_ids    = get_child_string_array (ctx, jp, 8,  "artist_ids");
  p->album         = get_child_string       (ctx, jp, 9,  "album");
  p->album_id      = get_child_string       (ctx, jp, 10, "album_id");
  p->album_artists = get_child_string_array (ctx, jp, 11, "album_artists");
  p->cover_url     = get_child_string       (ctx, jp, 12, "cover_url");
  p->url           = get_child_string       (ctx, jp, 13, "url");
  p->added_at      = get_child_string       (ctx, jp, 14, "added_at");
  p->list_index    = get_child_uint8_t      (ctx, jp, 15, "list_index");
  p->is_local      = get_child_boolean      (ctx, jp, 16, "is_local");
  p->is_playable   = get_child_boolean      (ctx, jp, 17, "is_playable");
}

void
free_player_message(PlayerMessage* pm)
{
  auto* p = &pm->playable;

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

  free(pm);
}

JsonMember*
get_child(JsonNode* parent, size_t index, const char* name)
{
  JsonMember* m = &parent->data.object_children[index];
  if (name != NULL) {
    assert(strcmp(m->key, name) == 0);
  }

  return m;
}

char*
get_child_string(Context* ctx, JsonMember* parent,
                 size_t index, const char* name)
{
  JsonMember* source_member = get_child(parent->value, index, name);

  char* string = strndup(source_member->value->data.string,
                         strlen(source_member->value->data.string));
  if (string == NULL) {
    handle_error(ctx, "get_child_string strndup");
  }

  return string;
}

double
get_child_number(Context* ctx, JsonMember* parent,
                 size_t index, const char* name)
{
  JsonMember* source_member = get_child(parent->value, index, name);

  return source_member->value->data.number;
}

uint32_t
get_child_uint32_t(Context* ctx, JsonMember* parent,
                   size_t index, const char* name)
{
  return (uint32_t)get_child_number(ctx, parent, index, name);
}

uint8_t
get_child_uint8_t(Context* ctx, JsonMember* parent,
                  size_t index, const char* name)
{
  return (uint8_t)get_child_number(ctx, parent, index, name);
}

bool
get_child_boolean(Context* ctx, JsonMember* parent,
                  size_t index, const char* name)
{
  JsonMember* source_member = get_child(parent->value, index, name);

  return source_member->value->data.boolean;
}

StringArray
get_child_string_array(Context* ctx, JsonMember* parent,
                       size_t index, const char* name)
{
  JsonMember* source_member = get_child(parent->value, index, name);
  assert(strcmp(source_member->key, name) == 0);

  size_t count = source_member->value->children_count;

  if (count == 0) {
    return (StringArray){ .data = NULL, .count = 0 };
  }

  StringArray array = {
    .data  = calloc(1, sizeof(char*) * count),
    .count = 0,
  };

  for (size_t i = 0; i < count; ++i) {
    char* source_string =
      source_member->value->data.array_children[i]->data.string;
    char* target_string = strndup(source_string, strlen(source_string));
    if (target_string == NULL) {
      handle_error(ctx, "get_child_string_array strndup");
    }

    array.data[i] = target_string;
    array.count  += 1;
  }

  return array;
}

PlayableType
get_child_playable_type(Context* ctx, JsonMember* parent,
                        size_t index, const char* name)
{
  JsonMember* source_member = get_child(parent->value, index, name);

  char* string = source_member->value->data.string;
  if (strcmp(string, "Track") == 0 ){
    return TRACK;
  }

  handle_error(ctx, "get_child_playable_type: unknown type: %s\n", string);

  return UNKNOWN_PLAYABLE_TYPE;
}

void
free_string_array(StringArray s)
{
  for (size_t i = 0; i < s.count; ++i) {
    free(s.data[i]);
  }
}

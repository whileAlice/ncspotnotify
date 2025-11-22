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
  PlayerMessage* pm           = calloc(1, sizeof(PlayerMessage));
  JsonNode*      json_message = json_parse(ctx, json_string);

  assert(json_message->type == OBJECT);

  JsonMember* mode = &json_message->data.object_children[0];
  assert(strcmp(mode->key, "mode") == 0);

  JsonMember* state = get_child(mode, 0);
  JsonMember* secs  = get_child(state, 0);
  JsonMember* nanos = get_child(state, 1);

  if        (strcmp(state->key, "Paused") == 0) {
    pm->mode.state = PAUSED;

    assert(strcmp(secs->key,  "secs") == 0);
    assert(strcmp(nanos->key, "nanos") == 0);

    pm->mode.secs  = (uint32_t)secs-> value->data.number;
    pm->mode.nanos = (uint32_t)nanos->value->data.number;
  } else if (strcmp(state->key, "Stopped") == 0) {
    pm->mode.state = STOPPED;
  } else if (strcmp(state->key, "Playing") == 0) {
    pm->mode.state = PLAYING;

    assert(strcmp(secs->key,  "secs_since_epoch"));
    assert(strcmp(nanos->key, "nanos_since_epoch"));

    pm->mode.secs_since_epoch  = (uint32_t)secs-> value->data.number;
    pm->mode.nanos_since_epoch = (uint32_t)nanos->value->data.number;
  } else {
    handle_error(ctx, "json_to_player_message: unknown mode state: %s\n", state);
  }

  JsonMember* playable = &json_message->data.object_children[1];
  assert(strcmp(playable->key, "playable") == 0);

  JsonMember* type = get_child(playable, 0);
  assert(strcmp(type->key, "type") == 0);

  if (strcmp(type->value->data.string, "Track") == 0) {
    pm->playable.type = TRACK;
  } else {
    handle_error(ctx, "json_to_player_message: unknown playable type: %s\n", state);
  }

  auto* p = &pm->playable;

  p->id            = get_child_string      (ctx, playable, 1,  "id");
  p->uri           = get_child_string      (ctx, playable, 2,  "uri");
  p->title         = get_child_string      (ctx, playable, 3,  "title");
  p->track_number  = get_child_uint8_t     (ctx, playable, 4,  "track_number");
  p->disc_number   = get_child_uint8_t     (ctx, playable, 5,  "disc_number");
  p->duration      = get_child_uint32_t    (ctx, playable, 6,  "duration");
  p->artists       = get_child_string_array(ctx, playable, 7,  "artists");
  p->artist_ids    = get_child_string_array(ctx, playable, 8,  "artist_ids");
  p->album         = get_child_string      (ctx, playable, 9,  "album");
  p->album_id      = get_child_string      (ctx, playable, 10, "album_id");
  p->album_artists = get_child_string_array(ctx, playable, 11, "album_artists");
  p->cover_url     = get_child_string      (ctx, playable, 12, "cover_url");
  p->url           = get_child_string      (ctx, playable, 13, "url");
  p->added_at      = get_child_string      (ctx, playable, 14, "added_at");
  p->list_index    = get_child_uint8_t     (ctx, playable, 15, "list_index");
  p->is_local      = get_child_boolean     (ctx, playable, 16, "is_local");
  p->is_playable   = get_child_boolean     (ctx, playable, 17, "is_playable");

  free_json_node(json_message);

  return pm;
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
get_child(JsonMember* parent, size_t index)
{
  return &parent->value->data.object_children[index];
}

char*
get_child_string(Context* ctx, JsonMember* parent,
                 size_t index, const char* name)
{
  JsonMember* source_member = get_child(parent, index);
  assert(strcmp(source_member->key, name) == 0);

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
  JsonMember* source_member = get_child(parent, index);
  assert(strcmp(source_member->key, name) == 0);

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
  JsonMember* source_member = get_child(parent, index);
  assert(strcmp(source_member->key, name) == 0);

  return source_member->value->data.boolean;
}

StringArray
get_child_string_array(Context* ctx, JsonMember* parent,
                       size_t index, const char* name)
{
  JsonMember* source_member = get_child(parent, index);
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
    char* source_string = source_member->value->data.array_children[i]->data.string;
    char* target_string = strndup(source_string, strlen(source_string));
    if (target_string == NULL) {
      handle_error(ctx, "get_member_child_string strndup");
    }

    array.data[i] = target_string;
    array.count  += 1;
  }

  return array;
}

void
free_string_array(StringArray s)
{
  for (size_t i = 0; i < s.count; ++i) {
    free(s.data[i]);
  }
}

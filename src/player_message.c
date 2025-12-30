#include "player_message.h"

#include "error.h"
#include "json_parse.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PlayerMessage*
json_to_player_message (char* json_string)
{
   PlayerMessage* pm = calloc (1, sizeof (PlayerMessage));
   if (pm == NULL)
   {
      set_error ("calloc");
      return NULL;
   }

   JsonNode* json = json_parse (json_string);
   if (json == NULL)
   {
      set_error ("json parse");
      return NULL;
   }

   assert (json->type == OBJECT);

   auto* json_mode = get_child (json, 0, "mode");
   pm->mode        = parse_json_mode (json_mode);
   if (pm->mode == NULL)
   {
      set_error ("parse json mode");
      return NULL;
   }

   auto* json_playable = get_child (json, 1, "playable");
   pm->playable        = parse_json_playable (json_playable);
   if (pm->playable == NULL)
   {
      set_error ("parse json playable");
      return NULL;
   }

   json_node_free (json);

   return pm;
}

PlayerMode*
parse_json_mode (JsonMember* json_mode)
{
   PlayerMode* m = calloc (1, sizeof (PlayerMode));
   if (m == NULL)
   {
      set_error ("calloc");
      return NULL;
   }

   // string mode
   if (json_mode->value->type == STRING)
   {
      if (strcmp (json_mode->value->data.string, "Stopped") == 0)
         m->state = STOPPED;
      else if (strcmp (json_mode->value->data.string, "FinishedTrack") == 0)
         m->state = FINISHED_TRACK;
      else
      {
         // FIXME: this never shows up when unknown string appears
         set_error ("unknown string mode state: '%s'",
                    json_mode->value->data.string);
         return NULL;
      }

      return m;
   }

   // k-v pair mode
   auto* state = get_child (json_mode->value, 0, NULL);

   if (strcmp (state->key, "Paused") == 0)
   {
      m->state = PAUSED;
      m->secs  = get_child_uint32_t (state, 0, "secs");
      m->nanos = get_child_uint32_t (state, 1, "nanos");
   }
   else if (strcmp (state->key, "Playing") == 0)
   {
      m->state             = PLAYING;
      m->secs_since_epoch  = get_child_uint32_t (state, 0, "secs_since_epoch");
      m->nanos_since_epoch = get_child_uint32_t (state, 1, "nanos_since_epoch");
   }
   else
   {
      set_error ("unknown k-v mode state key: '%s'", state->key);
      return NULL;
   }

   return m;
}

PlayerPlayable*
parse_json_playable (JsonMember* json_playable)
{
   PlayerPlayable* p = calloc (1, sizeof (PlayerPlayable));
   if (p == NULL)
   {
      set_error ("calloc");
      return NULL;
   }

   if (json_playable->value->type == JSON_NULL)
      return p;

   auto* jp = json_playable;

   p->type = get_child_playable_type (jp, 0, "type");
   if (p->type == UNKNOWN_PLAYABLE_TYPE)
   {
      set_error ("get child playable type");
      return NULL;
   }

   p->id           = get_child_string (jp, 1, "id");
   p->uri          = get_child_string (jp, 2, "uri");
   p->title        = get_child_string (jp, 3, "title");
   p->track_number = get_child_uint8_t (jp, 4, "track_number");
   p->disc_number  = get_child_uint8_t (jp, 5, "disc_number");
   p->duration     = get_child_uint32_t (jp, 6, "duration");

   p->artists = get_child_string_array (jp, 7, "artists");
   if (p->artists.data == NULL)
   {
      set_error ("artists: get child string array");
      return NULL;
   }

   p->artist_ids = get_child_string_array (jp, 8, "artist_ids");
   if (p->artist_ids.data == NULL)
   {
      set_error ("artist_ids: get child string array");
      return NULL;
   }

   p->album    = get_child_string (jp, 9, "album");
   p->album_id = get_child_string (jp, 10, "album_id");

   p->album_artists = get_child_string_array (jp, 11, "album_artists");
   if (p->album_artists.data == NULL)
   {
      set_error ("album_artists: get child string array");
      return NULL;
   }

   p->cover_url   = get_child_string (jp, 12, "cover_url");
   p->url         = get_child_string (jp, 13, "url");
   p->added_at    = get_child_string (jp, 14, "added_at");
   p->list_index  = get_child_uint32_t (jp, 15, "list_index");
   p->is_local    = get_child_boolean (jp, 16, "is_local");
   p->is_playable = get_child_boolean (jp, 17, "is_playable");

   return p;
}

void
player_message_free (PlayerMessage* pm)
{
   if (pm == NULL)
      return;

   auto* p = pm->playable;

   free (pm->mode);
   free (p->id);
   free (p->uri);
   free (p->title);
   string_array_free (p->artists);
   string_array_free (p->artist_ids);
   free (p->album);
   free (p->album_id);
   string_array_free (p->album_artists);
   free (p->cover_url);
   free (p->url);
   free (p->added_at);
   free (p);

   free (pm);
}

JsonMember*
get_child (JsonNode* parent, size_t index, const char* name)
{
   assert (parent->type == OBJECT);
   assert (parent->children_count >= index + 1);

   JsonMember* m = &parent->data.object_children[index];
   if (name != NULL)
      assert (strcmp (m->key, name) == 0);

   return m;
}

char*
get_child_string (JsonMember* parent, size_t index, const char* name)
{
   JsonMember* source_member = get_child (parent->value, index, name);
   assert (source_member->value->type == STRING);

   char* string                      = source_member->value->data.string;
   source_member->value->data.string = NULL;

   return string;
}

double
get_child_number (JsonMember* parent, size_t index, const char* name)
{
   JsonMember* source_member = get_child (parent->value, index, name);
   assert (source_member->value->type == NUMBER);

   return source_member->value->data.number;
}

uint32_t
get_child_uint32_t (JsonMember* parent, size_t index, const char* name)
{
   return (uint32_t)get_child_number (parent, index, name);
}

uint8_t
get_child_uint8_t (JsonMember* parent, size_t index, const char* name)
{
   return (uint8_t)get_child_number (parent, index, name);
}

bool
get_child_boolean (JsonMember* parent, size_t index, const char* name)
{
   JsonMember* source_member = get_child (parent->value, index, name);
   assert (source_member->value->type == BOOLEAN);

   return source_member->value->data.boolean;
}

StringArray
get_child_string_array (JsonMember* parent, size_t index, const char* name)
{
   JsonMember* source_member = get_child (parent->value, index, name);
   assert (strcmp (source_member->key, name) == 0);
   assert (source_member->value->type == ARRAY);

   size_t count = source_member->value->children_count;

   if (count == 0)
      return (StringArray){ .data = NULL, .count = 0 };

   StringArray array = {
      .data  = calloc (1, sizeof (char*) * count),
      .count = 0,
   };
   if (array.data == NULL)
   {
      set_error ("calloc");
      return array;
   }

   for (size_t i = 0; i < count; ++i)
   {
      auto* source_child = source_member->value->data.array_children[i];
      assert (source_child->type == STRING);

      array.data[i]  = source_child->data.string;
      array.count   += 1;

      source_child->data.string = NULL;
   }

   return array;
}

PlayableType
get_child_playable_type (JsonMember* parent, size_t index, const char* name)
{
   JsonMember* source_member = get_child (parent->value, index, name);
   assert (source_member->value->type == STRING);

   char* string = source_member->value->data.string;
   if (strcmp (string, "Track") == 0)
      return TRACK;

   set_error ("unknown type: '%s'", string);
   return UNKNOWN_PLAYABLE_TYPE;
}

void
string_array_free (StringArray s)
{
   for (size_t i = 0; i < s.count; ++i)
      free (s.data[i]);

   free (s.data);
}

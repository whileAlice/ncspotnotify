#include <stdint.h>

#include "context.h"
#include "json_parse.h"

typedef enum player_state {
  PAUSED = 0,
  PLAYING,
  STOPPED,
} PlayerState;

typedef enum playable_type {
  TRACK = 0,
  UNKNOWN_PLAYABLE_TYPE,
} PlayableType;

typedef struct string_array {
  char** data;
  size_t count;
} StringArray;

typedef struct player_mode {
  PlayerState state;
  union {
    uint32_t secs;
    uint32_t secs_since_epoch;
  };
  union {
    uint32_t nanos;
    uint32_t nanos_since_epoch;
  };
} PlayerMode;

typedef struct player_playable {
  PlayableType type;
  char*        id;
  char*        uri;
  char*        title;
  uint8_t      track_number;
  uint8_t      disc_number;
  uint32_t     duration;
  StringArray  artists;
  StringArray  artist_ids;
  char*        album;
  char*        album_id;
  StringArray  album_artists;
  char*        cover_url;
  char*        url;
  char*        added_at;
  uint8_t      list_index;
  bool         is_local;
  bool         is_playable;
} PlayerPlayable;

typedef struct player_message {
  PlayerMode     mode;
  PlayerPlayable playable;
} PlayerMessage;

PlayerMessage*  json_to_player_message (Context* ctx, char* json_string);
void            parse_player_mode      (Context* ctx, PlayerMode* m,
                                        JsonMember* json_mode);
void            parse_player_playable  (Context* ctx, PlayerPlayable* p,
                                        JsonMember* json_playable);
void            free_player_message    (PlayerMessage* pm);
JsonMember*     get_child              (JsonNode* parent,   size_t index,
                                        const char* name);
char*           get_child_string       (Context* ctx, JsonMember* parent,
                                        size_t index, const char* name);
double          get_child_number       (Context* ctx, JsonMember* parent,
                                        size_t index, const char* name);
uint32_t        get_child_uint32_t     (Context* ctx, JsonMember* parent,
                                        size_t index, const char* name);
uint8_t         get_child_uint8_t      (Context* ctx, JsonMember* parent,
                                        size_t index, const char* name);
bool            get_child_boolean      (Context* ctx, JsonMember* parent,
                                        size_t index, const char* name);
StringArray     get_child_string_array (Context* ctx, JsonMember* parent,
                                        size_t index, const char* name);
PlayableType    get_child_playable_type(Context* ctx, JsonMember* parent,
                                        size_t index, const char* name);
void            free_string_array      (StringArray s);

#include <stdint.h>

typedef enum player_mode {
  PAUSED = 0,
  PLAYING,
  STOPPED,
} PlayerMode;

typedef enum playable_type {
  TRACK = 0,
} PlayableType;

typedef struct player_state {
  struct {
    PlayerMode mode;
    uint32_t   secs;
    uint32_t   nanos;
  } mode;
  struct {
    PlayableType type;
    const char*  id;
    const char*  uri;
    const char*  title;
    uint8_t      track_number;
    uint8_t      disc_number;
    uint32_t     duration;
    const char** artists;
    const char** artist_ids;
    const char*  album;
    const char*  album_id;
    const char** album_artists;
    const char*  cover_url;
    const char*  url;
    const char*  added_at;
    uint8_t      list_index;
    bool         is_local;
    bool         is_playable;
  } playable;
} PlayerState;

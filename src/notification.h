#pragma once

#include "player_message.h"

#include <stdint.h>
#include <sys/types.h>

typedef struct notification {
   PlayerState state;
   char*       artists;
   char*       title;
   char*       album;
   char*       cover_url;
   uint32_t    current_time_s;
   uint32_t    total_time_s;
} Notification;

// clang-format off
Notification* player_message_to_notification (PlayerMessage* message);
void          notification_free              (Notification* n);
// TODO: unify both gets
char*         get_top_notification_string    (Notification* n);
char*         get_bottom_notification_string (Notification* n);
char*         get_progress_bar               (Notification* n);
char*         get_state_symbol               (PlayerState state);
char*         artists_to_string              (StringArray artists);
// TODO: think about cleaning the cover cache
char*         download_cover                 (char* cover_url);

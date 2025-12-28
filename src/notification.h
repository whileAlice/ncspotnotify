#pragma once

#include "player_message.h"

#include <stdint.h>

typedef struct notification {
   PlayerState state;
   char*       artists;
   char*       title;
   char*       album;
   uint32_t    current_time_s;
   uint32_t    total_time_s;
} Notification;

// clang-format off
Notification* player_message_to_notification (PlayerMessage* message);
char*         notification_to_string         (Notification* notification);
char*         get_state_symbol               (PlayerState state);
char*         artists_to_string              (StringArray artists);

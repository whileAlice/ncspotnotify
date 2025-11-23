#pragma once

#include <stdint.h>

#include "player_message.h"

typedef struct notification {
  PlayerState state;
  char*       artists;
  char*       title;
  char*       album;
  uint32_t    current_time_s;
  uint32_t    total_time_s;
} Notification;

Notification* player_message_to_notification(PlayerMessage* message);
void          free_notification             (Notification* n);
char*         notification_to_string        (Context* ctx, Notification* notification);
char*         get_state_symbol              (Context* ctx, PlayerState state);
char*         artists_to_string             (StringArray artists);

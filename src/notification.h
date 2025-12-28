#pragma once

#include <stdint.h>

#include "context.h"
#include "player_message.h"

typedef struct notification {
   PlayerState state;
   char*       artists;
   char*       title;
   char*       album;
   uint32_t    current_time_s;
   uint32_t    total_time_s;
} Notification;

typedef struct notifications {
   Notification** data;
   size_t         count;
   size_t         head;
   size_t         tail;
   size_t         capacity;
} Notifications;

Notification*  player_message_to_notification (PlayerMessage* message);
Notification*  clone_notification (Notification* n);
void           free_notification (Notification* n);
Notifications* init_notifications (void);
void           free_notifications (Notifications* ns);
void           enqueue_notification (Notifications* ns, Notification* n);
Notification*  dequeue_notification (Notifications* ns);
char*          notification_to_string (Notification* notification);
char*          get_state_symbol (PlayerState state);
char*          artists_to_string (StringArray artists);

#pragma once

#include <stdint.h>

#include "player_message.h"
#include "context.h"

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

Notification*  player_message_to_notification(Context* ctx, PlayerMessage* message);
void           free_notification             (Notification* n);
Notification*  clone_notification            (Context* ctx, Notification* n);
Notifications* init_notifications            (Context* ctx);
void           free_notifications            (Notifications* ns);
void           enqueue_notification          (Context* ctx, Notifications* ns, Notification* n);
Notification*  dequeue_notification          (Context* ctx, Notifications* ns);
char*          notification_to_string        (Context* ctx, Notification* notification);
char*          get_state_symbol              (Context* ctx, PlayerState state);
char*          artists_to_string             (StringArray artists);

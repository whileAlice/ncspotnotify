#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "notification.h"
#include "player_message.h"
#include "error.h"

#define INITIAL_NOTIFICATIONS_CAPACITY 5
#define MAX_NOTIFICATIONS_CAPACITY     20

Notification*
player_message_to_notification(PlayerMessage* message)
{
  assert(message->playable != NULL);

  Notification* n = calloc(1, sizeof(Notification));
  if (n == NULL) {
    handle_error("player_message_to_notification calloc");
  }

  n->state   = message->mode->state;
  n->artists = artists_to_string(message->playable->artists);
  n->title   = strdup(message->playable->title);
  n->album   = strdup(message->playable->album);

  return n;
}

void
free_notification(Notification* n)
{
  free(n->artists);
  free(n->title);
  free(n->album);

  free(n);
}

Notification*
clone_notification(Notification* n)
{
  Notification* c = malloc(sizeof(Notification));
  if (c == NULL) {
    handle_error("clone_notification malloc");
  }

  c->state   = n->state;
  c->artists = strdup(n->artists);
  if (c->artists == NULL) {
    handle_error("clone_notification artists strdup");
  }
  c->title   = strdup(n->title);
  if (c->title == NULL) {
    handle_error("clone_notification title strdup");
  }
  c->album   = strdup(n->album);
  if (c->album == NULL) {
    handle_error("clone_notification album strdup");
  }

  return c;
}

Notifications*
init_notifications(void)
{
  Notifications* ns = malloc(sizeof(Notifications));
  if (ns == NULL) {
    handle_error("init_notifications malloc");
  }

  ns->data     = calloc(INITIAL_NOTIFICATIONS_CAPACITY, sizeof(Notification*));
  if (ns->data == NULL) {
    handle_error("init_notifications calloc");
  }
  ns->count    = 0;
  ns->head     = 0;
  ns->tail     = 0;
  ns->capacity = INITIAL_NOTIFICATIONS_CAPACITY;

  return ns;
}

void
free_notifications(Notifications* ns)
{
  for (size_t i = 0; i < ns->count; ++i) {
    free_notification(ns->data[i]);
  }

  free(ns);
}

void
enqueue_notification(Notifications* ns, Notification* n)
{
  if (ns->capacity == ns->count && ns->capacity < MAX_NOTIFICATIONS_CAPACITY) {
    size_t new_cap = ns->capacity * sizeof(Notifications) * 2;

    ns = realloc(ns, new_cap);
    ns->capacity = new_cap;
  }

  if (ns->data[ns->tail] != NULL) {
    free_notification(ns->data[ns->tail]);
  }

  ns->data[ns->tail] = clone_notification(n);

  ns->tail   = (ns->tail + 1) % ns->capacity;
  ns->count += 1;
}

Notification*
dequeue_notification(Notifications* ns)
{
  assert(ns->data[ns->head] != NULL);

  Notification* n = clone_notification(ns->data[ns->head]);

  ns->head   = (ns->head + 1) % ns->capacity;
  ns->count -= 1;

  return n;
}

char*
notification_to_string(Notification* n)
{
  char* state  = get_state_symbol(n->state);

  int length = snprintf(NULL, 0, NOTIFICATION_FORMAT, state,
                        n->artists, n->title, n->album);

  char* str = malloc(length * sizeof(char));
  if (str == NULL) {
    handle_error("notification_to_string malloc");
  }

  if (snprintf(str, 1024, NOTIFICATION_FORMAT, state,
               n->artists, n->title, n->album) != length) {
    handle_error("notification_to_string snprintf");
  }

  return str;
}

char*
get_state_symbol(PlayerState state)
{
  char buf[4];

  switch (state) {
  case PAUSED:
    strcpy(buf, "\u23f8");
    break;
  case PLAYING:
    strcpy(buf, "\u23f5");
    break;
  case STOPPED:
    strcpy(buf, "\u23f9");
    break;
  }

  buf[3] = '\0';

  char* str = strdup(buf);
  if (str == NULL) {
    handle_error("notification get_state_symbol strdup");
  }

  return str;
}

char*
artists_to_string(StringArray artists)
{
  if (artists.count < 1) {
    return strdup("Unknown Artist");
  }

  size_t length = (artists.count - 1) * strlen(", ") + 1;
  for (size_t i = 0; i < artists.count; ++i) {
    length += strlen(artists.data[i]);
  }

  char* str = calloc(length, sizeof(char));

  strcat(str, artists.data[0]);

  for (size_t i = 1; i < artists.count; ++i) {
    strcat(str, ", ");
    strcat(str, artists.data[i]);
  }

  return str;
}

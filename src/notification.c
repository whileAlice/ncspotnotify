#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#define _POSIX_C_SOURCE 200809L
#include <string.h>

#include "notification.h"
#include "player_message.h"

Notification*
player_message_to_notification(PlayerMessage* message)
{
  Notification* n = calloc(1, sizeof(Notification));

  n->state   = message->mode.state;
  n->artists = artists_to_string(message->playable.artists);
  n->title   = strdup(message->playable.title);
  n->album   = strdup(message->playable.album);

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

char*
notification_to_string(Context* ctx, Notification* n)
{
  char* state = get_state_symbol(ctx, n->state);

  size_t length = strlen(state) + 1 +
                  strlen(n->artists) + 1 +
                  strlen("-") + 1 +
                  strlen(n->title) + 1 +
                  strlen("(") + strlen(n->album) + strlen(")") + 1;

  char* str = malloc(length * sizeof(char));

  strcat(str, state);
  strcat(str, " ");
  strcat(str, n->artists);
  strcat(str, " ");
  strcat(str, "-");
  strcat(str, " ");
  strcat(str, n->title);
  strcat(str, " ");
  strcat(str, "(");
  strcat(str, n->album);
  strcat(str, ")");

  str[length] = '\0';

  return str;
}

char*
get_state_symbol(Context* ctx, PlayerState state)
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
    handle_error(ctx, "notification get_state_symbol strdup");
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

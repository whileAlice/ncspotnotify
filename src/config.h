#pragma once

#define MESSAGE_BUFFER_SIZE 2048
// the format is: <state> <artists> - <track> (<album>)
#define NOTIFICATION_FORMAT "%s %s - %s (%s)"
#define NOTIFICATION_CMD    "dunstify"
#define IMAGE_PATH_SWITCH   "-I"
#define UNKNOWN_ARTIST      "Unknown Artist"
// TODO: respect $XDG_CACHE_HOME
#define CACHE_DIR           ".cache"
#define NCSPOTNOTIFY_DIR    "ncspotnotify"
// TODO: is this certain?
#define COVER_EXTENSION     "jpeg"
#define FILENAME_LENGTH     16
// $HOME/.cache/ncspotnotify/[FILENAME_LENGTH-char filename].jpeg
#define COVER_PATH_MASK     "%s/%s/%s/%s.%s"
// notification command timeout in seconds
#define CMD_TIMEOUT         2
// TODO: unhardcode this
#define SOCKET_PATH         "/run/user/1000/ncspot/ncspot.sock"

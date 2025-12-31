#pragma once

#define MESSAGE_BUFFER_SIZE 2048
// TODO: split into three strings and configure in dunstrc
// <title>
#define NOTIFICATION_TOP    "%s"
// <artists>\n<i><album></i>
#define NOTIFICATION_BOTTOM "%s\n<i>%s</i>"
#define NOTIFICATION_CMD    "dunstify"
#define IMAGE_PATH_SWITCH   "-I"
#define HINT_SWITCH         "-h"
// TODO: make it switchable (turn off if 0%?)
#define PROGRESS_BAR_HINT   "int:value:%d"
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

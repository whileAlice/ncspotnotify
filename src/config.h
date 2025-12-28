#pragma once

#define MESSAGE_BUFFER_SIZE 2048
// the format is: state artists - track (album)
#define NOTIFICATION_FORMAT "%s %s - %s (%s)"
#define NOTIFICATION_CMD    "dunstify"
// notification command timeout in seconds
#define CMD_TIMEOUT         2
// TODO: unhardcode this
#define SOCKET_PATH         "/run/user/1000/ncspot/ncspot.sock"

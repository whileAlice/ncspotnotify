#include "notifications.h"

#include "notification.h"
#include "ring_buffer.h"

RING_BUFFER_DEFINE (Notification*, notification, Notifications, notifications,
                    5, 20);

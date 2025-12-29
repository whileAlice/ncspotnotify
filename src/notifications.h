#pragma once

#include "notification.h"
#include "ring_buffer.h"

// clang-format off
Notification* notification_zero (void);
Notification* notification_copy (Notification* n);
void          notification_free (Notification* n);

RING_BUFFER_DECLARE (Notification*, notification,
                     Notifications, notifications);

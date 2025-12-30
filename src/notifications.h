#pragma once

#include "notification.h"
#include "ring_buffer.h"

void notification_free (Notification* n);

RING_BUFFER_DECLARE (Notification*, notification, Notifications, notifications);

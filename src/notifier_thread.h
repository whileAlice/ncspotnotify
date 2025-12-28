#pragma once

#include "notification.h"

// clang-format off
void* notifier_thread  (void* args);
void  log_notification (Notification* n);

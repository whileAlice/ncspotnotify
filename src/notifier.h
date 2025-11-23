#pragma once

#include "notification.h"
#include "context.h"

void* notifier_thread  (void* args);
void  send_notification(Context* ctx, Notification* n);

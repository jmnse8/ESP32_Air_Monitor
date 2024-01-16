#ifndef __PROVISIONING_HANDLER
#define __PROVISIONING_HANDLER

#include "esp_event.h"

void provisioning_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);

#endif
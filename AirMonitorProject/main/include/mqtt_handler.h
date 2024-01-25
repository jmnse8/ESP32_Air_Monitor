#ifndef __MQTT_HANDLER
#define __MQTT_HANDLER

#include "esp_event.h"

void mqtt_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);
void mqtt_init();

#endif
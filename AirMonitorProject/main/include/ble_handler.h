#ifndef __BLE_HANDLER
#define __BLE_HANDLER

#include "esp_event.h"

void ble_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);

#endif
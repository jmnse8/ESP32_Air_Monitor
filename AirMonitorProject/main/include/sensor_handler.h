
#include "esp_event.h"
#include "c_sensorSI7021.h"
#include "c_sensorSGP30.h"

void sensorSI7021_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);
void sensorSGP30_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);
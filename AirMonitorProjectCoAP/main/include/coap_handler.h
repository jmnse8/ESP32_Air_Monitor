#include "esp_event.h"

void coap_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);
void coap_init();
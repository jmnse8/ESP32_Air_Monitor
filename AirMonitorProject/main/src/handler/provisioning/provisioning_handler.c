#include "provisioning_handler.h"
#include "c_provisioning.h"

#include "esp_log.h"

static const char *TAG = "provisioning handler";

void provisioning_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    
    switch(id){
        case C_PROVISIONG_EVENT_CUSTOM_DATA:
            ESP_LOGI(TAG, "C_PROVISIONG_EVENT_CUSTOM_DATA");
        break;
        case C_PROVISIONG_EVENT_CONNECTED:
            ESP_LOGI(TAG, "C_PROVISIONG_EVENT_CONNECTED");
        break;
        
    }
}
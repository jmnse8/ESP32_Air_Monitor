#include "sensor_handler.h"

static const char* TAG = "SENSOR_HANDLER";

void sensorSI7021_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data){
   
    float value = *((float*)event_data);
    
     if(id==SENSORSI7021_HUM_DATA){
            ESP_LOGI(TAG, "HUM: %f",  value);
        }
        else{
            ESP_LOGI(TAG, "TEMP: %f",  value);
        }
}

void sensorSGP30_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data){
   
    uint16_t value = *((uint16_t*)event_data);
    
     if(id==SENSORSGP30_TVOC_DATA){
            ESP_LOGI(TAG, "TVOC: %d",  value);
        }
        else{
            ESP_LOGI(TAG, "ECO2: %d",  value);
        }
}
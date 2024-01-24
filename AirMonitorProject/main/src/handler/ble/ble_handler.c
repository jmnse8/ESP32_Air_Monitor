#include "ble_handler.h"
#include "esp_log.h"
#include "c_ble.h"
#include "c_mqtt.h"
#include "cJSON.h"

static const char* TAG = "BLE_HANDLER";

static void _headCount_data_handler(char *headCount){
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "headCount", headCount);
    char *data = cJSON_PrintUnformatted(root);
    mqtt_publish_to_topic(CONFIG_TB_TELEMETRY_TOPIC, (uint8_t*)data, strlen(data));
    cJSON_Delete(root);
}

void ble_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    
    switch(id){
        case C_BLE_EVENT_ATTENDANCE:
            char * headCount = (char *)event_data;
            ESP_LOGI(TAG, "RECEIVED ATTENDANCE DATA headCount=%s", headCount);
            _headCount_data_handler(headCount);
        break;
        
        default:
            ESP_LOGE(TAG, "UNKNOWN BLE EVENT ID: %ld", id);
            //ota_update_chunk_received(mqtt_data->data, mqtt_data->data_len);
            break;
            
 

    }
}

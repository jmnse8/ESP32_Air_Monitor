#include "sensor_handler.h"
#include "cJSON.h"
#include "c_mqtt.h"

static const char* TAG = "SENSOR_HANDLER";


char * float2str(float mfloat, int precision){

    // Determine the length needed for the formatted string
    int length = snprintf(NULL, 0, "%.*f", precision, mfloat);

    // Allocate a string with enough space
    char* formattedString = (char*)malloc(length + 1); // +1 for null terminator

    // Use snprintf to convert to a string with the specified precision
    snprintf(formattedString, length + 1, "%.*f", precision, mfloat);

    return formattedString;

}

void sensorSI7021_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data){
   
    float value = *((float*)event_data);
    cJSON *root = cJSON_CreateObject();
    
    if(id==SENSORSI7021_HUM_DATA){
        ESP_LOGI(TAG, "HUM: %f",  value);
        //cJSON_AddNumberToObject(root, "humidity", value);
        cJSON_AddStringToObject(root, "humidity", float2str(value, 2));

    }
    else{
        ESP_LOGI(TAG, "TEMP: %f",  value);
        //cJSON_AddNumberToObject(root, "temperature", value);
        cJSON_AddStringToObject(root, "temperature", float2str(value, 2));
    }
    char *data = cJSON_Print(root);
    mqtt_publish_to_topic(CONFIG_TB_TELEMETRY_TOPIC, (uint8_t*)data, strlen(data));

    free((void*)data);
    cJSON_Delete(root);
}

void sensorSGP30_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data){
   
    uint16_t value = *((uint16_t*)event_data);
    cJSON *root = cJSON_CreateObject();
    
    if(id==SENSORSGP30_TVOC_DATA){
        ESP_LOGI(TAG, "TVOC: %d",  value);
        cJSON_AddStringToObject(root, "tvoc", float2str(value, 2));
    }
    else{
        ESP_LOGI(TAG, "ECO2: %d",  value);
        cJSON_AddStringToObject(root, "eco2", float2str(value, 2));
    }
    char *data = cJSON_Print(root);
    mqtt_publish_to_topic(CONFIG_TB_TELEMETRY_TOPIC, (uint8_t*)data, strlen(data));

    free((void*)data);
    cJSON_Delete(root);
}


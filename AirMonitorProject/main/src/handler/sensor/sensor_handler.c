#include "sensor_handler.h"
#include "cJSON.h"
#include "c_mqtt.h"
#include "mqtt_parser.h"

static const char* TAG = "SENSOR_HANDLER";

enum TB_SENSOR_PIN {
    TB_SENSOR_PIN_TMP = 1,
    TB_SENSOR_PIN_HUM = 2,
    TB_SENSOR_PIN_ECO2 = 3
};

enum TB_SENSOR_ERR {
    TB_SENSOR_ERR_INVALID_ARGUMENT

};

static char * float2str(float mfloat, int precision){

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

void handler_set_sensor_stat(char * payload, char* response_topic){

    cJSON* root = cJSON_Parse(payload);
    if (root != NULL){
        cJSON* paramsItem = cJSON_GetObjectItem(root, "params");

        if(paramsItem!=NULL){
            cJSON* pinItem = cJSON_GetObjectItem(paramsItem, "pin");
            cJSON *enabledItem = cJSON_GetObjectItem(paramsItem, "enabled");

            int pinValue = 1;
            int enabled = 0;
            // Extract pin as an integer
            if (cJSON_IsNumber(pinItem)) {
                pinValue = pinItem->valueint;
            }

            cJSON_bool enabledValue = cJSON_CreateTrue();
            // Extract enabled as a boolean
            if (cJSON_IsBool(enabledItem)) {
                enabledValue = cJSON_IsTrue(enabledItem);
                enabled = enabledValue ? 1 : 0;
            }

            switch (pinValue) {
                case  TB_SENSOR_PIN_TMP:
                    si7021_set_sensor_onoff(SENSORSI7021_TEMP_SENSOR, enabled);
                break;
                case  TB_SENSOR_PIN_HUM:
                    si7021_set_sensor_onoff(SENSORSI7021_HUM_SENSOR, enabled);            
                break;
                case  TB_SENSOR_PIN_ECO2:
                break;
                default:
                break;
            }

            cJSON *root = cJSON_CreateObject();
            char str[10];
            sprintf(str, "%d", pinValue);
            cJSON_AddBoolToObject(root, str, enabledValue);

            
            char *data = cJSON_Print(root);
            mqtt_publish_to_topic(response_topic, (uint8_t*)data, strlen(data));
        }
        
    }

    cJSON_Delete(root);    
}

void handler_get_sensor_stat(char *request_id){

    int mode = get_sensor_mode_si7021();

    cJSON *root = cJSON_CreateObject();

    int status[4] = {0,0,0,0};

    switch(mode){
        case SENSORSI7021_TEMP_MODE:
            status[0] = 1;
        break;
        case SENSORSI7021_HUM_MODE:
            status[1] = 1;
        break;
        case SENSORSI7021_ALL_MODE:
            status[0] = 1;
            status[1] = 1;
        break;
    }

    printf("%d %d %d %d\n", status[0], status[1], status[2], status[3]);
    cJSON_AddBoolToObject(root, "1", status[0]);
    cJSON_AddBoolToObject(root, "2", status[1]);
    cJSON_AddBoolToObject(root, "3", status[2]);
    
    /*
    SENSORSI7021_DISABLED_MODE = 0
    SENSORSI7021_TEMP_MODE,
    SENSORSI7021_HUM_MODE,
    SENSORSI7021_ALL_MODE,
    */
    
    char *data = cJSON_Print(root);
    mqtt_publish_to_topic(build_topic("v1/devices/me/rpc/response/", request_id), (uint8_t*)data, strlen(data));

    free((void*)data);
    cJSON_Delete(root);
}

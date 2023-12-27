
#include <stdio.h>
#include <regex.h>
#include "esp_log.h"

#include "mqtt_handler.h"
#include "c_sensorSGP30.h"
#include "c_mqtt.h"

#include "mqtt_parser.h"
#include "context.h"
#include "cJSON.h"

static const char* TAG = "MQTT_MANAGER";
//char *NODE_CONTEXT_MAIN = "2/3";


static void freq_topic_handler(char *data){
    int res = parse_int_value(data);
    if (res > 0) {
        //change_sample_period_sgp30(res);
        ESP_LOGI(TAG, "FREQ value is %d", res);
    } else {
        ESP_LOGE(TAG, "FREQ value is invalid: %s", data);
    }
}


static void onoff_topic_handler(char *data){
    switch (parse_bool_value(data)) {
        case 0:
            ESP_LOGI(TAG, "SENSOR OFF");
            //stop_sensor_sgp30();
            break;
        case 1:
            ESP_LOGI(TAG, "SENSOR ON");
            //start_sensor_sgp30();
            break;
        default:
            ESP_LOGE(TAG, "ONOFF value is invalid: %s", data);
    }
}

/*
    mosquitto_pub -d -q 1 
    -h 147.96.85.120 
    -p 1883 
    -t v1/devices/me/telemetry 
    -u "XzscdzDfTPH3Xs4JGbkH" 
    -m "{temperature:25}"
*/
void mqtt_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    
    switch(id){
        case C_MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT CONNECTED");
            mqtt_subscribe_to_topic("v1/devices/me/rpc/request/+");
            //mqtt_subscribe_to_topic(build_topic(context_get_node_ctx(), "/+"));

        break;
        case C_MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT DISCONNECTED");
        break;
        case C_MQTT_EVENT_RECEIVED_DATA:
            struct mqtt_com_data* mqtt_data = (struct mqtt_com_data*)event_data;
            ESP_LOGI(TAG, "MQTT Received data %s from topìc %s", mqtt_data->data, mqtt_data->topic);
            int res;
            //if(its_for_me(mqtt_data->data)==0){
                switch(parse_method(mqtt_data->data)){
                    case MQTT_GET_FREQ_TOPIC:
                    break;
                    case MQTT_GET_ONOFF_TOPIC:
                        char * request_id = mqtt_topic_last_token(mqtt_data->topic);
                        int onoff = context_get_onoff();

                        cJSON *root = cJSON_CreateObject();
                        cJSON_AddBoolToObject(root, "onoff", cJSON_CreateBool(onoff));

                        char *data = cJSON_Print(root);
                        printf("%s\n", data);
                        mqtt_publish_to_topic(build_topic("v1/devices/me/rpc/response/", request_id), (uint8_t*)data, strlen(data));

                        free((void*)data);
                        free(request_id);
                        cJSON_Delete(root);

                    break;
                    case MQTT_GET_MODE_TOPIC:
                    break;
                    case MQTT_SET_FREQ_TOPIC:
                        freq_topic_handler(mqtt_data->data);
                        break;
                    case MQTT_SET_ONOFF_TOPIC:
                        onoff_topic_handler(mqtt_data->data);
                        
                        break;
                    case MQTT_SET_MODE_TOPIC:
                        res = parse_int_data(mqtt_data->data);
                        ESP_LOGI(TAG, "SENSOR MODE SET TO %d", res);
                        //set_sensor_mode_sgp30(res);
                       
                        break;

                    default:
                        ESP_LOGE(TAG, "UNKNOWN TOPIC: %s", mqtt_data->topic);
                    break;
                }
            //}
        break;

    }
}


void mqtt_init(){
    mqtt_set_qos(1);
    _mqtt_init(context_get_node_ctx());
}

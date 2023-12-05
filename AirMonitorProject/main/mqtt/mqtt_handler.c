
#include <stdio.h>
#include <regex.h>
#include "esp_log.h"

#include "c_mqtt.h"
#include "mqtt_handler.h"
#include "c_sensorSGP30.h"

#include "mqtt_parser.h"


static const char* TAG = "MQTT_MANAGER";

static void mqtt_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    switch(id){
        case C_MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT CONNECTED");
            subscribe_to_topic("3/2/TMP/+");
            subscribe_to_topic("3/2/+");
            publish_to_topic("3/2/ONOFF", (uint8_t *)"10");
        break;
        case C_MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT DISCONNECTED");
        break;
        case C_MQTT_EVENT_RECEIVED_DATA:
            struct mqtt_com_data* mqtt_data = (struct mqtt_com_data*)event_data;
            ESP_LOGI(TAG, "MQTT Received data %s from topìc %s", mqtt_data->data, mqtt_data->topic);
            
            switch(mqtt_topic_parser(mqtt_data->topic)){
                case C_MQTT_FREQ_TOPIC:
                    int res = parse_int_data(mqtt_data->data);
                    if (res > 0) {
                        //change_sample_period_sgp30(res);
                        ESP_LOGI(TAG, "FREQ value is %d", res);
                    } else {
                        ESP_LOGE(TAG, "FREQ value is invalid: %s", mqtt_data->data);
                    }
                    
                break;
                case C_MQTT_ONOFF_TOPIC:
                    switch (parse_int_data(mqtt_data->data)) {
                        case 0:
                            ESP_LOGI(TAG, "SENSOR OFF");
                            //stop_sensor_sgp30();
                            break;
                        case 1:
                            ESP_LOGI(TAG, "SENSOR ON");
                            //start_sensor_sgp30();
                            break;
                        default:
                            ESP_LOGE(TAG, "ONOFF value is invalid: %s", mqtt_data->data);
                    }
                break;

                default:
                    ESP_LOGE(TAG, "UNKNOWN TOPIC: %s", mqtt_data->topic);
                break;
            }
        break;

    }
}

void c_mqtt_init_event_handler(){

    ESP_ERROR_CHECK(esp_event_handler_register(C_MQTT_EVENT_BASE, C_MQTT_EVENT_CONNECTED, mqtt_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(C_MQTT_EVENT_BASE, C_MQTT_EVENT_DISCONNECTED, mqtt_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(C_MQTT_EVENT_BASE, C_MQTT_EVENT_RECEIVED_DATA, mqtt_handler, NULL));

}



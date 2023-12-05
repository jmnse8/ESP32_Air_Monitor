#include <stdio.h>
#include "esp_log.h"
#include "esp_event.h"
#include "c_sensorSI7021.h"
#include "c_sensorSGP30.h"
#include "c_wifiConnection.h"

#include "c_mqtt.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"

static const char* TAG = "main";

static void sensorSI7021_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data){
   
    float value = *((float*)event_data);
    
     if(id==SENSORSI7021_HUM_DATA){
            ESP_LOGI(TAG, "HUM: %f",  value);
        }
        else{
            ESP_LOGI(TAG, "TEMP: %f",  value);
        }
}

static void sensorSGP30_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data){
   
    uint16_t value = *((uint16_t*)event_data);
    
     if(id==SENSORSGP30_TVOC_DATA){
            ESP_LOGI(TAG, "TVOC: %d",  value);
        }
        else{
            ESP_LOGI(TAG, "ECO2: %d",  value);
        }
}

static void mqtt_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    switch(id){
        case MQTT_COM_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT CONNECTED");
            subscribe_to_topic("okok/hi");
            publish_to_topic("okok/hi", (uint8_t *)"HIYA");
        break;
        case MQTT_COM_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT DISCONNECTED");
        break;
        case MQTT_COM_EVENT_RECEIVED_DATA:
            struct mqtt_com_data* mqtt_data = (struct mqtt_com_data*)event_data;
            ESP_LOGI(TAG, "MQTT Received data %s from topìc %s", mqtt_data->data, mqtt_data->topic);
        break;

    }
}

void init_event_handlers(){

    ESP_ERROR_CHECK(esp_event_handler_register(MQTT_COM_EVENT_BASE, MQTT_COM_EVENT_CONNECTED, mqtt_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(MQTT_COM_EVENT_BASE, MQTT_COM_EVENT_DISCONNECTED, mqtt_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(MQTT_COM_EVENT_BASE, MQTT_COM_EVENT_RECEIVED_DATA, mqtt_handler, NULL));

    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSI7021_EVENT_BASE, SENSORSI7021_TEMP_DATA, sensorSI7021_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSI7021_EVENT_BASE, SENSORSI7021_HUM_DATA, sensorSI7021_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSGP30_EVENT_BASE, SENSORSGP30_TVOC_DATA, sensorSGP30_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSGP30_EVENT_BASE, SENSORSGP30_ECO2_DATA, sensorSGP30_event_handler, NULL));
    
}


void setup(){
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init();
    init_mqtt();

    init_event_handlers();

    init_sensor_si7021();
    init_sensor_sgp30();
}



void app_main(void){
    setup();
    while (1) vTaskDelay(1000 / portTICK_PERIOD_MS);
}
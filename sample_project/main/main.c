#include <stdio.h>
#include "mqtt_com.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include "protocol_examples_common.h"


static const char* TAG = "MAIN";


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

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    init_mqtt();

    ESP_ERROR_CHECK(esp_event_handler_register(MQTT_COM_EVENT_BASE, MQTT_COM_EVENT_CONNECTED, mqtt_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(MQTT_COM_EVENT_BASE, MQTT_COM_EVENT_DISCONNECTED, mqtt_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(MQTT_COM_EVENT_BASE, MQTT_COM_EVENT_RECEIVED_DATA, mqtt_handler, NULL));

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

#include <stdio.h>
#include <regex.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h"

#include "c_sensorSGP30.h"
#include "c_I2C.h"
#include "c_wifiConnection.h"
#include "c_sensorSI7021.h"
#include "c_mqtt.h"
#include "c_sntp.h"
#include "c_deepSleep.h"


#include "mqtt_handler.h"
#include "sensor_handler.h"


static void init_event_handlers(){

    //MQTT
    ESP_ERROR_CHECK(esp_event_handler_register(C_MQTT_EVENT_BASE, C_MQTT_EVENT_CONNECTED, mqtt_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(C_MQTT_EVENT_BASE, C_MQTT_EVENT_DISCONNECTED, mqtt_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(C_MQTT_EVENT_BASE, C_MQTT_EVENT_RECEIVED_DATA, mqtt_handler, NULL));

    //SENSORES
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSI7021_EVENT_BASE, SENSORSI7021_TEMP_DATA, sensorSI7021_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSI7021_EVENT_BASE, SENSORSI7021_HUM_DATA, sensorSI7021_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSGP30_EVENT_BASE, SENSORSGP30_TVOC_DATA, sensorSGP30_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSGP30_EVENT_BASE, SENSORSGP30_ECO2_DATA, sensorSGP30_event_handler, NULL));
    
}


void setup(){
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    init_event_handlers();
    wifi_init();
    mqtt_init();

    sntp_sync_time_init();
    init_deep_sleep();
    si7021_init_sensor();
    sgp30_init_sensor();
}


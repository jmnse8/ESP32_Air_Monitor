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
#include "c_coap.h"
#include "c_sntp.h"
#include "c_deepSleep.h"

#include "c_nvs.h"
#include "context.h"

#include "coap_handler.h"
#include "sensor_handler.h"


static void init_event_handlers(){

    //MQTT
    //ESP_ERROR_CHECK(esp_event_handler_register(C_MQTT_EVENT_BASE, C_MQTT_EVENT_CONNECTED, mqtt_handler, NULL));
    //ESP_ERROR_CHECK(esp_event_handler_register(C_MQTT_EVENT_BASE, C_MQTT_EVENT_DISCONNECTED, mqtt_handler, NULL));
    //ESP_ERROR_CHECK(esp_event_handler_register(C_MQTT_EVENT_BASE, C_MQTT_EVENT_RECEIVED_DATA, mqtt_handler, NULL));

    //CoAP
    ESP_ERROR_CHECK(esp_event_handler_register(C_COAP_EVENT_BASE, C_COAP_EVENT_RECEIVED_DATA, coap_handler, NULL));

    //SENSORES
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSI7021_EVENT_BASE, SENSORSI7021_TEMP_DATA, sensorSI7021_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSI7021_EVENT_BASE, SENSORSI7021_HUM_DATA, sensorSI7021_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSGP30_EVENT_BASE, SENSORSGP30_TVOC_DATA, sensorSGP30_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSGP30_EVENT_BASE, SENSORSGP30_ECO2_DATA, sensorSGP30_event_handler, NULL));
    
}


void setup(){
    nvs_init();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    context_refresh_node_status(NULL);

    init_event_handlers();
    wifi_init();
    coap_init();

    sntp_sync_time_init();
    init_deep_sleep();
    si7021_init_sensor();
    sgp30_init_sensor();
}
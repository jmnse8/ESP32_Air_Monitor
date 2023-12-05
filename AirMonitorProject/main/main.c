
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
#include "c_mqtt.h"

#include "mqtt_handler.h"

static const char* TAG = "MAIN";


void init_event_handlers(){
    c_mqtt_init_event_handler();
    
}


void setup(){
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init();
    init_mqtt();

    init_event_handlers();
}


void app_main(void){
    setup();
    while (1) vTaskDelay(1000 / portTICK_PERIOD_MS);
}
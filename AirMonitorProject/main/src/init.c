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
#include "c_provisioning.h"

#include "c_nvs.h"
#include "context.h"

#include "mqtt_handler.h"
#include "sensor_handler.h"
#include "provisioning_handler.h"
#include "ble_handler.h"

#include "c_spiffs.h"
#include "c_ble.h"

static void init_event_handlers(){

    //Provisioning
    ESP_ERROR_CHECK(esp_event_handler_register(C_PROVISIONING_EVENT_BASE, C_PROVISIONG_EVENT_CONNECTED, provisioning_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(C_PROVISIONING_EVENT_BASE, C_PROVISIONG_EVENT_DISCONNECTED, provisioning_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(C_PROVISIONING_EVENT_BASE, C_PROVISIONG_EVENT_CUSTOM_DATA, provisioning_handler, NULL));

    //MQTT
    ESP_ERROR_CHECK(esp_event_handler_register(C_MQTT_EVENT_BASE, C_MQTT_EVENT_CONNECTED, mqtt_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(C_MQTT_EVENT_BASE, C_MQTT_EVENT_DISCONNECTED, mqtt_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(C_MQTT_EVENT_BASE, C_MQTT_EVENT_RECEIVED_DATA, mqtt_handler, NULL));

    //SENSORES
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSI7021_EVENT_BASE, SENSORSI7021_TEMP_DATA, sensorSI7021_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSI7021_EVENT_BASE, SENSORSI7021_HUM_DATA, sensorSI7021_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSGP30_EVENT_BASE, SENSORSGP30_TVOC_DATA, sensorSGP30_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSGP30_EVENT_BASE, SENSORSGP30_ECO2_DATA, sensorSGP30_event_handler, NULL));

    // BLE
    ESP_ERROR_CHECK(esp_event_handler_register(C_BLE_EVENT_BASE, C_BLE_EVENT_ATTENDANCE, ble_handler, NULL));

}


void setup(){
    
    spiffs_init();
    esp_log_set_vprintf(&spiffs_log_vprintf);
    spiffs_activate_level2log(SPIFFS_LOGW);
    spiffs_activate_level2log(SPIFFS_LOGE);

    nvs_init();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    provisioning_init();

    int status = context_refresh_node_status(NULL);
    init_event_handlers();
    if(status!=NODE_STATE_PROV){
        mqtt_init();
        ble_init();
        si7021_init_sensor();
        sgp30_init_sensor();

        //sntp_sync_time_init();
        //init_deep_sleep();
    }
    

    
    
}


#include "init.h"


void setup(){
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    init_event_handlers();

    wifi_init();
    init_mqtt();

    init_sensor_si7021();
    init_sensor_sgp30();
}


void init_event_handlers(){

    //MQTT
    //ESP_ERROR_CHECK(esp_event_handler_register(MQTT_COM_EVENT_BASE, MQTT_COM_EVENT_CONNECTED, mqtt_handler, NULL));
    //ESP_ERROR_CHECK(esp_event_handler_register(MQTT_COM_EVENT_BASE, MQTT_COM_EVENT_DISCONNECTED, mqtt_handler, NULL));
    //ESP_ERROR_CHECK(esp_event_handler_register(MQTT_COM_EVENT_BASE, MQTT_COM_EVENT_RECEIVED_DATA, mqtt_handler, NULL));

    //SENSORES
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSI7021_EVENT_BASE, SENSORSI7021_TEMP_DATA, sensorSI7021_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSI7021_EVENT_BASE, SENSORSI7021_HUM_DATA, sensorSI7021_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSGP30_EVENT_BASE, SENSORSGP30_TVOC_DATA, sensorSGP30_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SENSORSGP30_EVENT_BASE, SENSORSGP30_ECO2_DATA, sensorSGP30_event_handler, NULL));
    
}
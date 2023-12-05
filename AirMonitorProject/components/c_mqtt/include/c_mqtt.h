#ifndef __MQTT_COM
#define __MQTT_COM

#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(MQTT_COM_EVENT_BASE);

extern int MQTT_STATUS;
extern int MQTT_QOS;

enum{
    MQTT_COM_EVENT_CONNECTED,
    MQTT_COM_EVENT_DISCONNECTED,
    MQTT_COM_EVENT_RECEIVED_DATA
};

struct mqtt_com_data{
    char* topic;
    char* data;
};

/*
    Initialize this component and establishes connection with the broker
    Make sure to call the following functions before:
        - ESP_ERROR_CHECK(nvs_flash_init());
        - ESP_ERROR_CHECK(esp_netif_init());
        - ESP_ERROR_CHECK(esp_event_loop_create_default());
        - ESP_ERROR_CHECK(example_connect());
*/
void init_mqtt();

/*
    If you don't want to use menuconfig to configure the mqtt broker
    Call this function before init_mqtt()
*/
int set_broker(char *new_broker);

/*
    Set qos of connection
        - QoS 0 = At most once (default)
        - QoS 1 = At least once
        - QoS 2 = Exactly once
*/
int set_qos(int q);


//Subscribe to topic
int subscribe_to_topic(char* topic);

//Unsubscribe to topic
int unsubscribe_to_topic(char* topic);

//Publish some data to a topic
int publish_to_topic(char* topic, uint8_t* data);

#endif
#ifndef __C_MQTT
#define __C_MQTT

#include "esp_event.h"
#include "mqtt_client.h"

ESP_EVENT_DECLARE_BASE(C_MQTT_EVENT_BASE);

extern int MQTT_STATUS;
extern int MQTT_QOS;
extern int MQTT_PORT;
extern char* MQTT_USERNAME;
extern char *MQTT_LWT_MESSAGE;

extern char* NODE_CONTEXT;

enum{
    C_MQTT_EVENT_CONNECTED,
    C_MQTT_EVENT_DISCONNECTED,
    C_MQTT_EVENT_RECEIVED_DATA
};

struct c_mqtt_data{
    char* topic;
    char* data;
    int data_len;
};

/*
    Initialize this component and establishes connection with the broker
    Make sure to call the following functions before:
        - ESP_ERROR_CHECK(nvs_flash_init());
        - ESP_ERROR_CHECK(esp_netif_init());
        - ESP_ERROR_CHECK(esp_event_loop_create_default());
        - ESP_ERROR_CHECK(example_connect());
*/
void mqtt_start_client();

void mqtt_stop_client();

/**
 * @brief Set MQTT Last Will Testament Message
*/
int mqtt_set_lwt_msg(char *msg);

/**
 * @brief Set MQTT port. 
 * Call this function before init_mqtt()
*/
int mqtt_set_port(int port);

/**
 * @brief Set MQTT username. 
 * Call this function before init_mqtt()
*/
int mqtt_set_username(char *username);

/**
 * @brief If you don't want to use menuconfig to configure the mqtt broker. 
 * Call this function before init_mqtt()
*/
int mqtt_set_broker(char *broker);

/**
 *  @brief  Set MQTT Quality of Service
 *  @param  qos
 *          QoS 0 = At most once (default), 
 *          QoS 1 = At least once, 
 *          QoS 2 = Exactly once
*/   
int mqtt_set_qos(int qos);

/**
 * @brief Provide the context data to the node --> "floor/room"
*/
void mqtt_set_context(char *c);

//Subscribe to topic
int mqtt_subscribe_to_topic(char* topic);

//Unsubscribe to topic
int mqtt_unsubscribe_to_topic(char* topic);

//Publish some data to a topic
int mqtt_publish_to_topic(char* topic, uint8_t* data, int data_length);

#endif
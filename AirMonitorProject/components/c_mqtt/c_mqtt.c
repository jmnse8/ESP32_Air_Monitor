#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "esp_err.h"
#include "mqtt_client.h"

#include "c_mqtt.h"


ESP_EVENT_DEFINE_BASE(C_MQTT_EVENT_BASE);

enum {
    MQTT_SETUP,
    MQTT_INIT,
    MQTT_CONNECTED,
    MQTT_ERR
};

int MQTT_STATUS = MQTT_SETUP;
int MQTT_QOS = 0;
char *MQTT_USERNAME = NULL;
char *MQTT_LWT_MESSAGE = NULL;
int MQTT_PORT = 1883;


static const char *TAG = "C_MQTT";
static char *MQTT_BROKER = CONFIG_MQTT_BROKER_URL;

static esp_mqtt_client_handle_t mqtt_client;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}


int mqtt_set_port(int port){
    if(MQTT_STATUS == MQTT_SETUP){
        MQTT_PORT = port;
        return 0;
    }
    return 1;
}

int mqtt_set_username(char *username){
    if(MQTT_STATUS == MQTT_SETUP){
        if(MQTT_USERNAME!=NULL)
            free(MQTT_USERNAME);

        int len = strlen(username);
        MQTT_USERNAME = malloc(len + 1);

        if (MQTT_USERNAME != NULL) {
            strncpy(MQTT_USERNAME, username, len);
            MQTT_USERNAME[len] = '\0'; 
            return 0;
        } 
    }
    return 1;
}

int mqtt_set_lwt_msg(char *msg){
    if(MQTT_STATUS == MQTT_SETUP){
        if(MQTT_LWT_MESSAGE!=NULL)
            free(MQTT_LWT_MESSAGE);

        int len = strlen(msg);
        MQTT_LWT_MESSAGE = malloc((len + 1) * sizeof(char));

        if (MQTT_LWT_MESSAGE != NULL) {
            strncpy(MQTT_LWT_MESSAGE, msg, len);
            MQTT_LWT_MESSAGE[len] = '\0'; 
            printf("\nMQTT_LWT_MESSAGE is %s\n", MQTT_LWT_MESSAGE);
            return 0;
        }else{
            ESP_LOGE(TAG, "eeeeeeeeeeeeeeeeeeeeee");
        }
    }
    return 1;
}

int mqtt_set_broker(char *broker){
    if(MQTT_STATUS == MQTT_SETUP){
        MQTT_BROKER = broker;
        return 0;
    }
    return 1;
}


int mqtt_set_qos(int qos){
    if(qos>-1 && qos<3){
        MQTT_QOS = qos;
        return 0;
    }
    return 1;
}

int mqtt_subscribe_to_topic(char* topic){
    if(MQTT_STATUS == MQTT_CONNECTED){
       int msg_id = esp_mqtt_client_subscribe(mqtt_client, topic, MQTT_QOS);
        ESP_LOGI(TAG, "sent subscribe with msg_id=%d and qos=%d", msg_id, MQTT_QOS);
        return 0;
    }

    return 1;
}

int mqtt_unsubscribe_to_topic(char* topic){
    if(MQTT_STATUS == MQTT_CONNECTED){
        int msg_id = esp_mqtt_client_unsubscribe(mqtt_client, topic);
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        return 0;
    }

    return 1;
}

int mqtt_publish_to_topic(char* topic, uint8_t* data, int data_length){
    if(MQTT_STATUS == MQTT_CONNECTED){
        int retain = 0;
        int msg_id = esp_mqtt_client_publish(mqtt_client, topic, (const void *)data, data_length, MQTT_QOS, retain);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        return 0;
    }

    return 1;
}


/*
    Put the received data into a struct and post it through the default event loop
*/
static void handle_received_data(const char* topic, int topic_len, const char* data, int data_len) {

    // Allocate memory for the struct
    struct mqtt_com_data* mqtt_data = malloc(sizeof(struct mqtt_com_data));
    if (mqtt_data == NULL) {
        // Handle allocation failure
        return;
    }

    // Allocate memory for the strings and copy data
    mqtt_data->topic = malloc(topic_len + 1);  // +1 for null terminator
    mqtt_data->data = malloc(data_len + 1);    // +1 for null terminator

    if (mqtt_data->topic == NULL || mqtt_data->data == NULL) {
        // Handle allocation failure
        free(mqtt_data->topic);
        free(mqtt_data->data);
        free(mqtt_data);
        return;
    }

    // Copy the data into the struct
    memcpy(mqtt_data->topic, topic, topic_len);
    mqtt_data->topic[topic_len] = '\0';  // Null-terminate the string
    memcpy(mqtt_data->data, data, data_len);
    mqtt_data->data[data_len] = '\0';    // Null-terminate the string

    // Post the event
    esp_event_post(C_MQTT_EVENT_BASE, C_MQTT_EVENT_RECEIVED_DATA, mqtt_data, sizeof(struct mqtt_com_data), 0);
}

/**
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {

        case MQTT_EVENT_CONNECTED:
            MQTT_STATUS = MQTT_CONNECTED;
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            esp_event_post(C_MQTT_EVENT_BASE, C_MQTT_EVENT_CONNECTED, NULL, 0, 0);
            break;

        case MQTT_EVENT_DISCONNECTED:
            MQTT_STATUS = MQTT_ERR;
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            esp_event_post(C_MQTT_EVENT_BASE, C_MQTT_EVENT_DISCONNECTED, NULL, 0, 0);
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            handle_received_data((const char*)event->topic, event->topic_len, (const char*)event->data, event->data_len);
            break;

        case MQTT_EVENT_ERROR:
            MQTT_STATUS = MQTT_ERR;
            ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

static void mqtt_start(){

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = MQTT_BROKER,
                .port = MQTT_PORT,
            }
        },
        .credentials = {
            .username = MQTT_USERNAME,
            //.username = "8nyHV2MCBKKqa7Mfs6sG",
            //.username = "0erTLZgiRFIiCzgn1AnT",
        },
        #ifdef CONFIG_MQTT_USE_LWT
        .session = {
            .last_will = {
                .topic = CONFIG_MQTT_LWT_TOPIC,
                #ifdef CONFIG_MQTT_USE_LWT_CUSTOM_MSG
                .msg = CONFIG_MQTT_LWT_MESSAGE,
                .msg_len = strlen(CONFIG_MQTT_LWT_MESSAGE),
                #else
                .msg = MQTT_LWT_MESSAGE,
                .msg_len = strlen(MQTT_LWT_MESSAGE),
                #endif
            },
            .keepalive = CONFIG_MQTT_LWT_KEEPALIVE,
        },
        #endif
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}


void mqtt_stop_client(){
    if (mqtt_client != NULL) {
        esp_mqtt_client_stop(mqtt_client);
        mqtt_client = NULL; // Set to NULL to avoid using a stopped client

        MQTT_STATUS = MQTT_SETUP;
        MQTT_QOS = 0;
        MQTT_USERNAME = NULL;
        MQTT_PORT = 1883;
    }
}

void mqtt_start_client(){
    if(mqtt_client == NULL && MQTT_STATUS == MQTT_SETUP){
        MQTT_STATUS = MQTT_INIT;
        /*
        esp_log_level_set("esp-tls", ESP_LOG_DEBUG);
        esp_log_level_set("transport", ESP_LOG_DEBUG);
        esp_log_level_set("mqtt_client", ESP_LOG_DEBUG);
        */
        mqtt_start();
    }
}

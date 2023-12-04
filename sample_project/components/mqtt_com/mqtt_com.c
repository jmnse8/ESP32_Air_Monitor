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

#include "mqtt_com.h"


ESP_EVENT_DEFINE_BASE(MQTT_COM_EVENT_BASE);

enum {
    MQTT_SETUP,
    MQTT_INIT,
    MQTT_CONNECTED,
    MQTT_ERR
};

int MQTT_STATUS = MQTT_SETUP;
int MQTT_QOS = 0;

static const char *TAG = "MQTT_COM";
static const char *TOPIC = "3/2/TMP";
static char *BROKER = CONFIG_BROKER_URL;

static esp_mqtt_client_handle_t mqtt_client;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}


int set_broker(char *new_broker){
    if(MQTT_STATUS == MQTT_SETUP){
        BROKER = new_broker;
        return 0;
    }
    return 1;
}


int set_qos(int q){
    if(q>-1 && q<3){
        MQTT_QOS = q;
        return 0;
    }
    return 1;
}

int subscribe_to_topic(char* topic){
    if(MQTT_STATUS == MQTT_CONNECTED){
       int msg_id = esp_mqtt_client_subscribe(mqtt_client, topic, MQTT_QOS);
        ESP_LOGI(TAG, "sent subscribe with msg_id=%d and qos=%d", msg_id, MQTT_QOS);
        return 0;
    }

    return 1;
}

int unsubscribe_to_topic(char* topic){
    if(MQTT_STATUS == MQTT_CONNECTED){
        int msg_id = esp_mqtt_client_unsubscribe(mqtt_client, topic);
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        return 0;
    }

    return 1;
}

int publish_to_topic(char* topic, uint8_t* data){
    if(MQTT_STATUS == MQTT_CONNECTED){
        int retain = 0;
        int msg_id = esp_mqtt_client_publish(mqtt_client, topic, (const void *)data, sizeof(data), MQTT_QOS, retain);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        return 0;
    }

    return 1;
}


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
    esp_event_post(MQTT_COM_EVENT_BASE, MQTT_COM_EVENT_RECEIVED_DATA, mqtt_data, sizeof(struct mqtt_com_data), 0);
}

/*
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
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        MQTT_STATUS = MQTT_CONNECTED;
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        esp_event_post(MQTT_COM_EVENT_BASE, MQTT_COM_EVENT_CONNECTED, NULL, 0, 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        MQTT_STATUS = MQTT_ERR;
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        esp_event_post(MQTT_COM_EVENT_BASE, MQTT_COM_EVENT_DISCONNECTED, NULL, 0, 0);
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
        //printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        //printf("DATA=%.*s\r\n", event->data_len, event->data);
        handle_received_data((const char*)event->topic, event->topic_len, (const char*)event->data, event->data_len);
        break;
    case MQTT_EVENT_ERROR:
        MQTT_STATUS = MQTT_ERR;
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
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

static void mqtt_start(void){
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = BROKER,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

void init_mqtt(){
    MQTT_STATUS = MQTT_INIT;

    esp_log_level_set("esp-tls", ESP_LOG_DEBUG);
    esp_log_level_set("transport", ESP_LOG_DEBUG);
    esp_log_level_set("mqtt_client", ESP_LOG_DEBUG);

/*
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);


    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());
*/
    mqtt_start();
}

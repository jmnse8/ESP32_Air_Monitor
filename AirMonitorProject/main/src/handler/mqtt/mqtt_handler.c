
#include <stdio.h>
#include <regex.h>
#include "esp_log.h"

#include "mqtt_handler.h"
#include "sensor_handler.h"
#include "c_sensorSGP30.h"
#include "c_mqtt.h"

#include "mqtt_parser.h"
#include "context.h"
#include "cJSON.h"
#include "ota_handler.h"
#include "mqtt_client_side_rpc.h"

static const char* TAG = "MQTT_HANDLER";


static void freq_topic_handler(char *data){
    int res = parse_params_int_value(data);
    if (res > 0) {
        //change_sample_period_sgp30(res);
        ESP_LOGI(TAG, "FREQ value is %d", res);
    } else {
        ESP_LOGE(TAG, "FREQ value is invalid: %s", data);
    }
}


static void _onoff_topic_handler(char *data){
    switch (parse_params_bool_value(data)) {
        case 0:
            ESP_LOGI(TAG, "SENSOR OFF");
            //stop_sensor_sgp30();
            break;
        case 1:
            ESP_LOGI(TAG, "SENSOR ON");
            //start_sensor_sgp30();
            break;
        default:
            ESP_LOGE(TAG, "ONOFF value is invalid: %s", data);
    }
}

static void _signup2tb(){
    mqtt_subscribe_to_topic(CONFIG_TB_PROVISION_RESPONSE_TOPIC);
    char * request = build_TB_prov_request();

    mqtt_publish_to_topic(CONFIG_TB_PROVISION_REQUEST_TOPIC, (uint8_t*)request, strlen(request));
    free(request);
}

static void _start_with_tb_token(char *token){
    mqtt_stop_client();
    mqtt_set_qos(1);
    mqtt_set_username(token);
    //mqtt_set_lwt_msg(context_get_node_ctx());

    cJSON *root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "alive", 0);
    cJSON_AddStringToObject(root, "reason", "LWT - Node disconnected");
    char *data = cJSON_Print(root);

    mqtt_set_lwt_msg(data);
    mqtt_start_client();
    context_refresh_node_status(NODE_STATE_REGULAR);
}



static void _get_access_token_TB(char *payload){
    char * token = get_access_token_TB_response(payload);

    if(token!=NULL){
        context_set_tb_access_token(token);
        _start_with_tb_token(token);
        free(token);
    }
}

static void _get_node_ctx_from_TB(char *payload){
    char *ctx;
    parse_params_str_value(payload, &ctx);
    context_set_node_ctx(ctx, 1);
    free(ctx);
}

static void _on_connected(){
    switch (context_get_node_status()) {
        case NODE_STATE_HAS_TB_TOKEN:
        case NODE_STATE_WAIT_CTX:
            _start_with_tb_token(context_get_tb_access_token());
            break;
        case NODE_STATE_SIGUP_DEVICE2TB:
            _signup2tb();
            break;
        case NODE_STATE_REGULAR:
            mqtt_subscribe_to_topic(CONFIG_TB_SS_RPC_REQUEST_TOPIC);
            mqtt_subscribe_to_topic(CONFIG_TB_CS_RPC_RESPONSE_TOPIC);
            //https://thingsboard.io/docs/reference/mqtt-api/
            mqtt_subscribe_to_topic("v1/devices/me/attributes/response/+");
            mqtt_subscribe_to_topic("v1/devices/me/attributes");

            mqtt_subscribe_to_topic("v2/sw/response/+");
            mqtt_subscribe_to_topic("v2/fw/response/+");

            
            request_publish_frequency();

            #ifdef CONFIG_MQTT_LWT_TOPIC
            mqtt_subscribe_to_topic(CONFIG_MQTT_LWT_TOPIC);
            #endif
            ESP_LOGI(TAG, "\n\n_on_connected ALL GUD");
            break;
        default:
            ESP_LOGI(TAG, "EH?");
            break;
    }
}


static void _get_onoff_status(char * request_id){
    int onoff = context_get_onoff();

    cJSON *root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "onoff", onoff);

    char *data = cJSON_Print(root);
    mqtt_publish_to_topic(build_topic(CONFIG_TB_SS_RPC_RESPONSE_TOPIC, request_id), (uint8_t*)data, strlen(data));

    free((void*)data);
    free(request_id);
    cJSON_Delete(root);
}


void rpc_request_handler(struct c_mqtt_data* mqtt_data){
    char * request_id;

    switch(parse_method(mqtt_data->data)){
        case MQTT_SET_CTX:
            _get_node_ctx_from_TB(mqtt_data->data);
            break;
        case MQTT_GET_SENSOR_STAT_TOPIC:
            handler_get_sensor_stat(mqtt_topic_last_token(mqtt_data->topic));
        break;
        case MQTT_SET_SENSOR_STAT_TOPIC:
            request_id = mqtt_topic_last_token(mqtt_data->topic);
            handler_set_sensor_stat(mqtt_data->data, build_topic(CONFIG_TB_SS_RPC_RESPONSE_TOPIC, request_id));
            free(request_id);
        break;
        case MQTT_GET_ONOFF_TOPIC:
            _get_onoff_status(mqtt_topic_last_token(mqtt_data->topic));
        break;
        case MQTT_SET_ONOFF_TOPIC:
            _onoff_topic_handler(mqtt_data->data);
            break;
        default:
            ESP_LOGE(TAG, "UNKNOWN RPC REQUEST: \nTOPIC=%s\nDATA=%s", mqtt_data->topic, mqtt_data->data);
        break;
    }
}


static const char *freq_topics[] = {"tmp_pub_freq", "hum_pub_freq", "eco2_pub_freq", "tvoc_pub_freq"};

static int _frequency_handler(char * payload){
    int res = MQTT_INVALID_VALUE;

    cJSON* root = cJSON_Parse(payload);
    if (root != NULL){

        for (size_t i = 0; i < 4; i++) {
            cJSON *item = cJSON_GetObjectItem(root, freq_topics[i]);
            if (item != NULL) {
                res = MQTT_SET_PUB_FREQ_TOPIC;
                handler_set_publish_frequency(item->valueint, i);
                break;
            }
        }
    }
    cJSON_Delete(root);
    return res;
}

static int _ota_update_handler(char * payload){
    
    int res = MQTT_INVALID_VALUE;

    cJSON* root = cJSON_Parse(payload);
    if (root != NULL){
        cJSON *sw_titleItem = cJSON_GetObjectItem(root, "sw_title");
        cJSON *fw_titleItem = cJSON_GetObjectItem(root, "fw_title");

        if(sw_titleItem!=NULL || fw_titleItem!=NULL){
            printf("\nES OTA\n");
            res = MQTT_OTA_UPDATE_SETUP;
            ota_incoming_update_handler(payload);
        }else{
            cJSON *deletedItem = cJSON_GetObjectItem(root, "deleted");
            if(deletedItem!=NULL){
                res = MQTT_OK;
            }
        }
    }
    cJSON_Delete(root);
    return res;
}



void rpc_response_handler(char * payload){
  cJSON* root = cJSON_Parse(payload);
    if (root != NULL){
        cJSON *typeItem = cJSON_GetObjectItem(root, "type");
        if(typeItem!=NULL){
            if(strcmp(typeItem->valuestring, "ALL_PUB_FREQ")==0){
                publish_frequency_response_handler(payload);
            }
            else if (strcmp(typeItem->valuestring, "CTX")==0){
                publish_frequency_response_handler(payload);
            }
        }
    }
   
}

void attributes_request_handler(char * payload){
    if(_ota_update_handler(payload)==MQTT_INVALID_VALUE){
        if(_frequency_handler(payload)==MQTT_INVALID_VALUE){

            ESP_LOGE(TAG, "UNKNOWN ATTRIBUTE REQUEST");
        }
    }
}


void mqtt_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    
    switch(id){
        case C_MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT CONNECTED");
            _on_connected();
        break;
        case C_MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT DISCONNECTED");
        break;
        case C_MQTT_EVENT_RECEIVED_DATA:

            //esp_mqtt_event_handle_t handler = event_data;
            struct c_mqtt_data* mqtt_data = (struct mqtt_com_data*)event_data;
            ESP_LOGI(TAG, "MQTT Received data %s from topic %s", mqtt_data->data, mqtt_data->topic);

            if (mqtt_data->data_len == 0) {
                ESP_LOGE(TAG, "\nhandler->data_len == 0\n");
                return;
            }

            switch(parse_topic(mqtt_data->topic)){
                /*
                    {
                    "sw_title":"aaa",
                    "sw_version":"1.0.1",
                    "sw_tag":"aaa 1.0.1",
                    "sw_size":175392,
                    "sw_checksum_algorithm":"SHA256",
                    "sw_checksum":"d50389bbc268d9fa040a44c389f56615f58c824c0df8277cda869c496cacc961"
                    } from topic v1/devices/me/attributes
                */
                case TB_TOPIC_ATTR_REQ:
                    ESP_LOGI(TAG, "Mensaje atributos MQTT:\n%s\n", mqtt_data->data);
                    attributes_request_handler(mqtt_data->data);
                    break;
                case TB_TOPIC_ATTR_RESP:
                    ESP_LOGI(TAG, "Mensaje atributos MQTT:\n%s\n", mqtt_data->data);
                    break;
                case TB_TOPIC_PROV_RESP:
                    /*
                    {
                        "credentialsValue":"8Of1Ees1Wa5nj2lEommm",
                        "credentialsType":"ACCESS_TOKEN",
                        "status":"SUCCESS"
                        } from topic /provision/response
                    */
                    if(parse_method(mqtt_data->data)==MQTT_SET_PROV_TOKEN)
                        _get_access_token_TB(mqtt_data->data);
                    break;

                case TB_TOPIC_RPC_REQ:
                /*
                    {
                        "method":"CTX",
                        "params":
                            {
                                "val":"1/2"
                            }
                    } from topic v1/devices/me/rpc/request/0
                */
                    rpc_request_handler(mqtt_data);
                    break;
                case TB_TOPIC_RPC_RESP:
                    ESP_LOGI(TAG, "Mensaje RPC:\n%s\n", mqtt_data->data);
                    rpc_response_handler(mqtt_data->data);
                    break;

                default:
                    ESP_LOGE(TAG, "UNKNOWN TOPIC: \nTOPIC=%s\nDATA=%s", mqtt_data->topic, mqtt_data->data);
                    //ota_update_chunk_received(mqtt_data->data, mqtt_data->data_len);
                    break;
            }
 
        break;

    }
}


void mqtt_init(){
    mqtt_set_qos(1);
    mqtt_set_username(CONFIG_TB_PROVISION_USERNAME);
    mqtt_set_lwt_msg(context_get_node_ctx());
    mqtt_start_client();
}

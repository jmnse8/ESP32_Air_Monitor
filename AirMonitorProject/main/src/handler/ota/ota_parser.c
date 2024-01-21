#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "esp_log.h"
#include "c_mqtt.h"

#include "cJSON.h"
#include "context.h"
#include "string.h"
#include "ota_handler.h"
#include "ota_parser.h"

static const char* TAG = "OTA_PARSER";

static const int N = 250;
int MQTT_UPDATE_REQUEST_ID = 1;

/**
 * VIA HTTP(S): http(s)://$THINGSBOARD_HOST_NAME/api/v1/$ACCESS_TOKEN/firmware?title=$TITLE&version=$VERSION
 * VIA MQTT: v2/fw/request/${requestId}/chunk/${chunkIndex} 
 * I (34025) MQTT_HANDLER: MQTT Received data 
 *      {
 *          "sw_title":"esp32_N-SGP30",
 *          "sw_version":"1.0.1",
 *          "sw_tag":"esp32_N-SGP30 1.0.1",
 *          "sw_size":959888,
 *          "sw_checksum_algorithm":"SHA256",
 *          "sw_checksum":"3ca9b827b4f134225f063b88b468ae211cb39846c6df461b028a495afd20d624"
 *      } from topìc v1/devices/me/attributes
*/
char *ota_build_update_request_topic(int req_id, int req_chunk){
    char topic[N];
    snprintf(topic, N, "v2/fw/request/%d/chunk/%d", req_id, req_chunk);

    printf("topic is %s with size %d\n",  topic, strlen(topic));

    char *res = (char *)malloc((strlen(topic) + 1) * sizeof(char));
    strncpy(res, topic, strlen(topic));

    return res;
}


char *ota_get_update_url(char *payload){
    char * url = NULL;
    cJSON* root = cJSON_Parse(payload);
    if (root != NULL){
        cJSON* sw_versionItem = cJSON_GetObjectItem(root, "sw_version");
        char *version = sw_versionItem->valuestring;

        if(context_check_sw_version(version)){
            cJSON* sw_urlItem = cJSON_GetObjectItem(root, "sw_url");
            char *res = sw_urlItem->valuestring;
            //char * https = "https://";
            int len = strlen(res);
            url = (char *)malloc((len + 1) * sizeof(char) );
            //strcpy(url, https);
            strcpy(url, res);
            url[len] = '\0';
        }
    }
    cJSON_Delete(root);
    return url;
}


int ota_get_update_size(char *payload){
    int len = -1;
    cJSON* root = cJSON_Parse(payload);
    if (root != NULL){
        /*
        cJSON* sw_versionItem = cJSON_GetObjectItem(root, "sw_version");
        char *version = sw_versionItem->valuestring;

        if(context_check_sw_version(version)){
            cJSON* sw_sizeItem = cJSON_GetObjectItem(root, "sw_size");
            len = sw_sizeItem->valueint;
        }
        */
       cJSON* fw_versionItem = cJSON_GetObjectItem(root, "fw_version");
        char *version = fw_versionItem->valuestring;

        if(context_check_sw_version(version)){
            cJSON* sw_sizeItem = cJSON_GetObjectItem(root, "fw_size");
            len = sw_sizeItem->valueint;
        }
        cJSON_Delete(root);
    }
    return len;
}

char *ota_build_update_request_url(char *payload){
    //http(s)://$THINGSBOARD_HOST_NAME/api/v1/$ACCESS_TOKEN/firmware?title=$TITLE&version=$VERSION
    char * res = NULL;
    cJSON* root = cJSON_Parse(payload);
    if (root != NULL){
        cJSON* sw_versionItem = cJSON_GetObjectItem(root, "sw_version");
        char *version = sw_versionItem->valuestring;

        if(context_check_sw_version(version)){
            cJSON* sw_titleItem = cJSON_GetObjectItem(root, "sw_title");
            char *title = sw_titleItem->valuestring;
            char * access_token = context_get_tb_access_token();
            char url[N];

            snprintf(url, N, "http://192.168.1.85:8080/api/v1/%s/firmware?title=%s&version=%s", access_token, title, version);

            res = (char *)malloc(strlen(url) * sizeof(char));
            strcpy(res, url);
            
        }
    }
    cJSON_Delete(root);
    return res;
}



void ota_update_status(char *status){
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "sw_state", status);
    char *payload = cJSON_PrintUnformatted(root);
    ota_send_status_update(payload);
    cJSON_free(root);
}


void ota_download_chunk(int chunk) {

    char payload_buffer[16];
    char topic_buffer[128];
    int n = snprintf(topic_buffer, sizeof(topic_buffer), "v2/fw/request/%d/chunk/%d", MQTT_UPDATE_REQUEST_ID, chunk);
    if (n >= sizeof(topic_buffer)) {
        ota_update_status("FAILED");
        ESP_LOGE(TAG, "Failed to download firmware: MQTT topic too long.");
        return;
    }

    n = snprintf(payload_buffer, sizeof(payload_buffer), "%lu", UPDATE_CHUNK_SIZE);
    if (n >= sizeof(topic_buffer)) {
        ota_update_status("FAILED");
        ESP_LOGE(TAG, "Chunk size buffer too small.");
        return;
    }
    
    mqtt_publish_to_topic(topic_buffer, (void *)payload_buffer, strlen(payload_buffer));
}



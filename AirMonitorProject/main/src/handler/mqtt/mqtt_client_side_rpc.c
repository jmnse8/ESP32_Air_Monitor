#include "cJSON.h"
#include "c_mqtt.h"
#include "mqtt_parser.h"
#include "sensor_handler.h"

#include "mqtt_client_side_rpc.h"

static int requestId = 0;

void publish_frequency_response_handler(char * payload){
    cJSON* root = cJSON_Parse(payload);
    if (root != NULL){
    
        cJSON *tmp_freqItem = cJSON_GetObjectItem(root, "tmp_freq");
        if(tmp_freqItem!=NULL){
            handler_set_publish_frequency(tmp_freqItem->valueint, SI7021_TMP);
        }
        cJSON *hum_freqItem = cJSON_GetObjectItem(root, "hum_freq");
        if(hum_freqItem!=NULL){
            handler_set_publish_frequency(hum_freqItem->valueint, SI7021_HUM);
        }
        cJSON *eco2_freqItem = cJSON_GetObjectItem(root, "eco2_freq");
        if(eco2_freqItem!=NULL){
            handler_set_publish_frequency(eco2_freqItem->valueint, SGP30_ECO2);
        }
        cJSON *tvoc_freqItem = cJSON_GetObjectItem(root, "tvoc_freq");
        if(tvoc_freqItem!=NULL){
            handler_set_publish_frequency(tvoc_freqItem->valueint, SGP30_TVOC);
        }
        
        
    }
    cJSON_Delete(root);

}

void request_publish_frequency(){
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "method", "G_PUB_FREQ");
    cJSON_AddItemToObject(root, "params", NULL);
    char *data = cJSON_Print(root);

    char buf[10];
    sprintf(buf, "%d", requestId);
    
    mqtt_publish_to_topic("v1/devices/me/rpc/request/1", (void *)data, strlen(data));
    //mqtt_publish_to_topic(build_topic(CONFIG_TB_CS_RPC_REQUEST_TOPIC, buf), NULL, 0);
    requestId++;
}

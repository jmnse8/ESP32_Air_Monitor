#include "provisioning_handler.h"
#include "c_provisioning.h"
#include "context.h"

#include "esp_log.h"
#include "cJSON.h"

static const char *TAG = "provisioning handler";

void provisioning_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    
    switch(id){
        case C_PROVISIONG_EVENT_CUSTOM_DATA:
            ESP_LOGI(TAG, "C_PROVISIONG_EVENT_CUSTOM_DATA");
            char *data = (char *)event_data;
            cJSON* root = cJSON_Parse(data);
            if (root != NULL){

                cJSON *acc_tokenItem = cJSON_GetObjectItem(root, "access_token");

                if(acc_tokenItem!=NULL){
                    char *token = acc_tokenItem->valuestring;
                    context_set_tb_access_token(token);
                    context_refresh_node_status(NODE_STATE_HAS_TB_TOKEN);
                }
            }
            cJSON_Delete(root);

        break;
        case C_PROVISIONG_EVENT_CONNECTED:
            ESP_LOGI(TAG, "C_PROVISIONG_EVENT_CONNECTED");
            context_refresh_node_status(NODE_STATE_HAS_WIFI_CREDENTIALS);
        break;
        case C_PROVISIONG_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "C_PROVISIONG_EVENT_DISCONNECTED");
            context_refresh_node_status(NODE_STATE_WIFI_DISCONNECTED);
        break;
    }
    //esp_restart();
}
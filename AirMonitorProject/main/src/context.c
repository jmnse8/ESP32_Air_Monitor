#include <stdio.h>
#include <string.h>
#include "context.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_check.h"

#include "c_nvs.h"
#include "c_provisioning.h"
#include "mqtt_client_side_rpc.h"

static const char* TAG = "CONTEXT";
static const char *AUX_CTX = "CONTEXTO PROVISIONAL";

static char *NODE_CONTEXT = NULL;
char *NODE_TB_ACCESS_TOKEN = NULL;

char *NODE_WIFI_SSID = NULL;
char *NODE_WIFI_PWD = NULL;

char* NODE_SW_VERSION = "0.0.0";
static int NODE_ONOFF_STATUS = CONTEXT_ON;

int NODE_STATUS = NODE_STATE_BLANK;

int context_check_sw_version(const char* ver){
    return strcmp(NODE_SW_VERSION, ver) < 0;
}

int context_is_invalid_ctx(){
    return strcmp(NODE_CONTEXT, AUX_CTX)==0;
}

void context_reset(){

    if(NODE_STATUS == NODE_STATE_INVALID_DATA){
        esp_err_t err;
        provisioning_erase_provision_data();
        
        err = nvs_delete_key(CONFIG_NVS_KEY_TB_CTX);
        if( err != ESP_OK ){
            ESP_LOGE(TAG, "Could not erase CONFIG_NVS_KEY_TB_CTX");
        }
        //reset
        esp_restart();
    }
}


void _esp_restart(){
    for(int i=10; i>0; i--){
        ESP_LOGI(TAG, "Restarting in %d", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
    esp_restart();
}


int context_refresh_node_status(int status){
    if(status==NULL) {

        NODE_STATUS = NODE_STATE_PROV;
        //char *wifi_ssid = context_get_wifi_ssid();
        //char *wifi_pwd = context_get_wifi_pwd();
        if(provisioning_is_provisioned()){
            NODE_STATUS = NODE_STATE_HAS_WIFI_CREDENTIALS;
            char *token = context_get_tb_access_token();

            if(token!=NULL){
                NODE_STATUS = NODE_STATE_HAS_TB_TOKEN;
            }
            else{
                NODE_STATUS = NODE_STATE_SIGUP_DEVICE2TB;
            }
        }

    }
    else {
        if(status >= 0){
            //In case it needs a reset
            if( (NODE_STATUS==NODE_STATE_HAS_TB_TOKEN && status==NODE_STATE_HAS_WIFI_CREDENTIALS)
                || (NODE_STATUS==NODE_STATE_HAS_WIFI_CREDENTIALS && status==NODE_STATE_HAS_TB_TOKEN) ){

                _esp_restart();
            }
            else{
                NODE_STATUS = status%NODE_CONTROL_PARSE;   
            }
        }
    }

    return NODE_STATUS;
}


void context_set_tb_access_token(char * token){
    printf("---------------------\n");
    printf("TOKEN = %s size %d\n", token, strlen(token));
    printf("---------------------\n");

    if(nvs_write_string(CONFIG_NVS_KEY_TB_TOKEN, token)!= NVS_OK){
        ESP_LOGE(TAG, "COULDN'T WRITE THE TOKEN %s WITH KEY %s", token, CONFIG_NVS_KEY_TB_TOKEN);
    } else {
        int len = strlen(token);
        NODE_TB_ACCESS_TOKEN =  (char *)malloc((len+1)*sizeof(char));
        strncpy(NODE_TB_ACCESS_TOKEN, token, len);
        NODE_TB_ACCESS_TOKEN[len] = '\0';
        //NODE_STATUS = NODE_STATE_REGULAR;
    }
}

int context_get_node_status(){
    return NODE_STATUS;
}

char *context_get_tb_access_token(){
    if(NODE_TB_ACCESS_TOKEN==NULL){
        if(nvs_read_string(CONFIG_NVS_KEY_TB_TOKEN, &NODE_TB_ACCESS_TOKEN)!=NVS_OK){
            ESP_LOGE(TAG, "COULDN'T READ THE TOKEN");
        }
        else{
            printf("NODE_TB_ACCESS_TOKEN = %s\n", NODE_TB_ACCESS_TOKEN);
        }
    }
    return NODE_TB_ACCESS_TOKEN;
}

void context_set_node_ctx(char *c, int save){
    if (c != NULL) {
        int len = strlen(c);

        if(NODE_CONTEXT!=NULL){
            free(NODE_CONTEXT);
        }
        NODE_CONTEXT = malloc((len + 1) * sizeof(char)); // +1 for the null terminator
        strncpy(NODE_CONTEXT, c, len);
        NODE_CONTEXT[len] = '\0';

        if(save)
            nvs_write_string(CONFIG_NVS_KEY_TB_CTX, NODE_CONTEXT);

        ESP_LOGI(TAG, "NODE_CONTEXT IS NOW %s", NODE_CONTEXT);
    }
}

char *context_get_node_ctx(){
    if(NODE_CONTEXT==NULL){
        if(nvs_read_string(CONFIG_NVS_KEY_TB_CTX, &NODE_CONTEXT)!=NVS_OK){
            ESP_LOGE(TAG, "Ctx is not in nvs.");
            context_set_node_ctx(AUX_CTX, 0);
        }
        else{
            ESP_LOGD(TAG, "NVS_KEY_TB_CTX = %s\n", NODE_CONTEXT);
        }
    }
    return NODE_CONTEXT;
}

int context_get_onoff(){
    return NODE_ONOFF_STATUS;
}

void context_set_onoff(int onoff){
    NODE_ONOFF_STATUS = onoff;
}



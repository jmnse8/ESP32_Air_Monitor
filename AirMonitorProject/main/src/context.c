#include <stdio.h>
#include <string.h>

#include "context.h"
#include "esp_log.h"
#include "c_nvs.h"

static const char* TAG = "CONTEXT";

static char *NODE_CONTEXT = NULL;
char *NODE_TB_TOKEN = NULL;
static int NODE_ONOFF_STATUS = CONTEXT_ON;

static const char* NVS_KEY_TB_TOKEN = "tb_prov_token";
static const char* NVS_KEY_TB_CTX = "tb_node_ctx";

int NODE_STATUS = NODE_STATE_SIGUP_DEVICE2TB_STATE;



void context_refresh_node_status(int status){
    if(status==NULL) {
        char *token = context_get_node_tb_token();
        if(token!=NULL){
            NODE_STATUS = NODE_STATE_HAS_TB_TOKEN;
        }
        else{
            NODE_STATUS = NODE_STATE_SIGUP_DEVICE2TB_STATE;
        }
    }
    else {
        if(status==NODE_STATE_BLANK
            || status==NODE_STATE_PROV_WIFI
            || status==NODE_STATE_SIGUP_DEVICE2TB_STATE
            || status==NODE_STATE_HAS_TB_TOKEN
            || status==NODE_STATE_REGULAR){
                NODE_STATUS = status;
            }
    }
    
}   

void context_set_node_tb_token(char * token){
    printf("---------------------\n");
    printf("TOKEN = %s size %d\n", token, strlen(token));
    printf("---------------------\n");
    if(NODE_STATUS == NODE_STATE_SIGUP_DEVICE2TB_STATE){
        if(nvs_write_string(NVS_KEY_TB_TOKEN, token)!= NVS_OK){
            ESP_LOGE(TAG, "COULDN'T WRITE THE TOKEN %s WITH KEY %s", token, NVS_KEY_TB_TOKEN);
        } else {
            int len = strlen(token);
            NODE_TB_TOKEN =  (char *)malloc((len+1)*sizeof(char));
            strncpy(NODE_TB_TOKEN, token, len);
            NODE_TB_TOKEN[len] = '\0';
            NODE_STATUS = NODE_STATE_REGULAR;
        }
    }
   
}

int context_get_node_status(){
    return NODE_STATUS;
}

char *context_get_node_tb_token(){
    if(NODE_TB_TOKEN==NULL){
        if(nvs_read_string(NVS_KEY_TB_TOKEN, &NODE_TB_TOKEN)!=NVS_OK){
            ESP_LOGE(TAG, "COULDN'T READ THE TOKEN");
        }
        else{
            printf("NODE_TB_TOKEN = %s\n", NODE_TB_TOKEN);
        }
    }
    return NODE_TB_TOKEN;
}

void context_set_node_ctx(char *c){
    if (c != NULL) {
        int len = strlen(c);

        if(NODE_CONTEXT!=NULL){
            free(NODE_CONTEXT);
        }
        NODE_CONTEXT = malloc((len + 1) * sizeof(char)); // +1 for the null terminator
        strncpy(NODE_CONTEXT, c, len);
        NODE_CONTEXT[len] = '\0';

        nvs_write_string(NVS_KEY_TB_CTX, NODE_CONTEXT);

        ESP_LOGI(TAG, "\n NODE_CONTEXT IS NOW %s\n", NODE_CONTEXT);
    }
}

char *context_get_node_ctx(){
    if(NODE_CONTEXT==NULL){
        if(nvs_read_string(NVS_KEY_TB_CTX, &NODE_CONTEXT)!=NVS_OK){
            if(NODE_STATUS==NODE_STATE_REGULAR)
                ESP_LOGE(TAG, "Ctx is not in nvs. Setting provisional value to: CONTEXTO PROVISIONAL");
            context_set_node_ctx("CONTEXTO PROVISIONAL");
        }
        else{
            ESP_LOGD(TAG, "NVS_KEY_TB_CTX = %s\n", NODE_CONTEXT);
        }
    }
    return NODE_CONTEXT;
}

int context_it_is_i(char * ctx){
    return strncmp(NODE_CONTEXT, ctx, strlen(NODE_CONTEXT));
}

int context_get_onoff(){
    return NODE_ONOFF_STATUS;
}

void context_set_onoff(int onoff){
    NODE_ONOFF_STATUS = onoff;
}



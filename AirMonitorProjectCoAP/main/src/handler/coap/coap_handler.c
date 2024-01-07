#include "cJSON.h"
#include "context.h"
#include "esp_log.h"
#include "coap_handler.h"
#include "c_coap.h"
#include <string.h>

char *get_access_token_TB_responseC(char *payload);
static void _get_access_token_TB(char *payload);

void coap_init(){
    coap_start_client();
}

void coap_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    char *payload = (char *)event_data;
    _get_access_token_TB(payload);
}

static void _get_access_token_TB(char *payload){
    char * token = get_access_token_TB_responseC(payload);

    if(token!=NULL){
        context_set_node_tb_token(token);
        //_start_with_tb_token(token);
        free(token);
    }
}
char *get_access_token_TB_responseC(char *payload){
    cJSON* root = cJSON_Parse(payload);
    char *acc_token = NULL;
    char *expected_status = "SUCCESS";
    if (root != NULL){
        cJSON* statusItem = cJSON_GetObjectItem(root, "status");
        char *status = statusItem->valuestring;
        if(strncmp(expected_status, status, strlen(expected_status))==0){
            cJSON* tokenItem = cJSON_GetObjectItem(root, "credentialsValue");
            char *aux = tokenItem->valuestring;
            acc_token = malloc(strlen(aux)+1);
            strncpy(acc_token, aux, strlen(aux));
            acc_token[strlen(aux)] = '\0';
        }
    }
    cJSON_Delete(root);
    return acc_token;
}
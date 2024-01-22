#include "cJSON.h"
#include "context.h"
#include "esp_log.h"
#include "coap_handler.h"
#include "c_coap.h"
#include <string.h>

char *get_access_token_TB_responseC(char *payload);
static void _get_access_token_TB(char *payload);
char * build_TB_prov_request();

void coap_init(){
    coap_start_client();
    char * token = context_get_node_tb_token();
    if (token == NULL) {
        ESP_LOGI("coapHND", "El token de nvs era nulo");
        coap_client_provision_send(build_TB_prov_request());
    }
    else {
        ESP_LOGI("coapHND", "El token de nvs NO era nulo %s", token);
        save_device_token(token);
    }
    free(token);
}

void coap_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    char *payload = (char *)event_data;
    _get_access_token_TB(payload);
}

static void _get_access_token_TB(char *payload){
    char * token = get_access_token_TB_responseC(payload);

    if(token!=NULL){
        context_set_node_tb_token(token);
        save_device_token(token);
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

char * build_TB_prov_request(){
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "deviceName", "esp32Coap5");
    cJSON_AddStringToObject(root, "provisionDeviceKey", "d01bk1lk8fsi5gr67xdl");
    cJSON_AddStringToObject(root, "provisionDeviceSecret", "5kln5xcz73euzgecvu6o");

    char *data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return data;
} 
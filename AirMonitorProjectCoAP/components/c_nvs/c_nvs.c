/* Non-Volatile Storage (NVS) Read and Write a Value - Example

   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"


#include "c_nvs.h"
#include <string.h>
#include "esp_log.h"

static const char* TAG = "NVS_COM";

static nvs_handle_t _nvs_handle;

static void _nvs_open_storage(){
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &_nvs_handle);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
}


static void _nvs_init(){
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
}

static int _nvs_write_string(char *key, char *value){
    _nvs_open_storage();

    int ret = NVS_ERR;
    esp_err_t err = nvs_set_blob(_nvs_handle, key, value, strlen(value));
    if(err == ESP_OK){
        ESP_LOGI(TAG, "Se ha escrito en NVS: %s:%s", key, value);
        err = nvs_commit(_nvs_handle);
        if(err == ESP_OK){
            ESP_LOGI(TAG, "COMMITED IN NVS");
            ret = NVS_OK;
        }
        else{
            ESP_LOGE(TAG, "FAILED TO COMMIT IN NVS");
        }
    }
    else{
        ESP_LOGE(TAG, "Ha fallado en escribir en NVS: %s:%s", key, value);
    }
    
    nvs_close(_nvs_handle);

    return ret;
}

static int _nvs_read_string(char *key, char **str){
    _nvs_open_storage();

    size_t len = 0;     // value will default to 0, if not set yet in NVS
    int ret = NVS_OK;
    esp_err_t err = nvs_get_blob(_nvs_handle, key, NULL, &len);

    switch (err) {
        case ESP_OK:
            
            *str = (char *)malloc((len+1) * sizeof(char));

            if (*str != NULL) {
                if(len > 0){
                    err = nvs_get_blob(_nvs_handle, key, *str, &len);
                    (*str)[len] = '\0';
                }
            }
            else { ret = NVS_ERR; }
            break;

        case ESP_ERR_NVS_NOT_FOUND:
            ret = NVS_KEY_NOT_FOUND;
            break;

        default :
            ESP_LOGE(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
            ret = NVS_ERR;
            break;
    }
    nvs_close(_nvs_handle);
    return ret;
}

void nvs_init(){
    _nvs_init();
}

int nvs_write_string(char *key, char *value){
    return _nvs_write_string(key, value);
}

int nvs_read_string(char *key, char **str){
    return _nvs_read_string(key, str);
}
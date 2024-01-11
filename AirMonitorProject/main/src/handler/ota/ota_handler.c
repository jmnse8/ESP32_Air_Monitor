#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_event.h"

#include "esp_log.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"

#include "ota_parser.h"
#include "c_mqtt.h"
#include <math.h>

static const char* TAG = "OTA_HANDLER";
int REQUEST_CHUNK = 0;
int REQUEST_ID = 321654987;
int file_size = 0;
static int downloading = 0;
static uint32_t n_chunks_downloaded = 0;
static uint32_t n_total_chunks = 0;
esp_ota_handle_t ota_handle;
esp_partition_t *update_partition = NULL;


extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

static char *UPDATE_URL;


enum{
    OTA_OK,
    OTA_ERR
};


static bool _diagnostic(void) {
    ESP_LOGI(TAG, "Diagnostics (5 sec)...");
    //vTaskDelay(5000 / portTICK_PERIOD_MS);
    return 1;
}


void ota_check() {
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            bool diagnostic_is_ok = _diagnostic();
            if (diagnostic_is_ok) {
                ESP_LOGI(TAG, "Diagnostics completed successfully! Continuing execution ...");
                esp_ota_mark_app_valid_cancel_rollback();
            } else {
                ESP_LOGE(TAG, "Diagnostics failed! Start rollback to the previous version ...");
                esp_ota_mark_app_invalid_rollback_and_reboot();
            }
        } 
        else{
            ESP_LOGI("MAIN", "ota_state != ESP_OTA_IMG_PENDING_VERIFY");
        }
    }

    printf("\n----------------------------\nOTA VERIFIED!\n");
    fflush(stdout);
    
    //esp_restart();
}


esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}


//int _download_update(char *url){
void _download_update(void *pvParameter){
    ESP_LOGI(TAG, "Starting OTA _download_update");
    ota_update_status("DOWNLOADING");

    esp_http_client_config_t config = {
        .url = UPDATE_URL,
        #ifdef CONFIG_OTA_USE_CERT_BUNDLE
        .crt_bundle_attach = esp_crt_bundle_attach,
        #else
        .cert_pem = (char *)server_cert_pem_start,
        #endif 
        .event_handler = _http_event_handler,
        .keep_alive_enable = true,
        #ifdef CONFIG_OTA_FIRMWARE_UPGRADE_BIND_IF
        .if_name = &ifr,
        #endif
    };

    #ifdef CONFIG_OTA_SKIP_COMMON_NAME_CHECK
    config.skip_cert_common_name_check = true;
    #endif

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    ESP_LOGI(TAG, "Attempting to download update from %s", config.url);

    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA Succeed, Rebooting...");
        ota_update_status("DOWNLOADED");
        ota_update_status("UPDATING");
        esp_restart();
    } else {
        ESP_LOGE(TAG, "Firmware upgrade failed");
        ota_update_status("FAILED");

    }
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}








/**
 * @brief later expand  it with CONFIG_ to choose between mqtt or coap
*/
void ota_send_status_update(char * payload){
    mqtt_publish_to_topic(CONFIG_TB_TELEMETRY_TOPIC, (void *)payload, strlen(payload));
}


esp_err_t ota_update_downloaded() {

    esp_err_t err;

    ESP_LOGI(TAG, "Actualizacion descargada");
    ota_update_status("DOWNLOADED");

    err = esp_ota_end(ota_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            ESP_LOGE(TAG, "Image validation failed, image is corrupted");
        } else {
            ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
        }
        ota_update_status("FAILED");
        return err;
    }

    ota_update_status("UPDATING");

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        return err;
    }
    ESP_LOGI(TAG, "Prepare to restart system!");
    esp_restart();
    
    return ESP_OK;
}

void ota_update_chunk_received(char *data, int data_len) {
    //ESP_ERROR_CHECK(esp_ota_write_with_offset(ota_handle, data, data_len, UPDATE_CHUNK_SIZE * n_chunks_downloaded));
    ESP_ERROR_CHECK(esp_ota_write(ota_handle, data, data_len));
    n_chunks_downloaded += 1;
    ESP_LOGI(TAG, "Downloaded update chunk %lu/%lu", n_chunks_downloaded, n_total_chunks);
    file_size += data_len;
    if (n_chunks_downloaded < n_total_chunks)
        ota_download_chunk(n_chunks_downloaded);
    else 
        ESP_ERROR_CHECK(ota_update_downloaded());
}


void ota_start_update(int size) {

    esp_err_t err;
    update_partition = esp_ota_get_next_update_partition(NULL);
 
    if (update_partition == NULL) {
        ESP_LOGE(TAG, "Error en esp_ota_get_next_update_partition");
        return;
    }

    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error en esp_ota_begin");
        return;
    }

    ESP_LOGI(TAG, "Iniciando actualizacion. Nueva versión: %d bytes", size);
    downloading = 1;
    file_size = size;
    n_chunks_downloaded = 0;
    n_total_chunks = (int) ceil((double) size / (double) UPDATE_CHUNK_SIZE);
    
    ota_update_status("DOWNLOADING");
    ota_download_chunk(n_chunks_downloaded);
}


void ota_incoming_update_handler(char * payload){
    printf("\n__________\n");
    /*
    int size = ota_get_update_size(payload);
    ota_start_update(size);
    */
   UPDATE_URL = ota_get_update_url(payload);
   xTaskCreate(&_download_update, "_download_update_task", 8192, NULL, 5, NULL);
   //_download_update(url);
}






















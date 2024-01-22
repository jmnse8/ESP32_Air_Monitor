#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "esp_bt.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "esp_event.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

ESP_EVENT_DECLARE_BASE(C_BLE_EVENT_BASE);

enum{
    C_BLE_EVENT_ATTENDANCE,
    C_BLE_EVENT_CONNECTED,
};

void ble_init();
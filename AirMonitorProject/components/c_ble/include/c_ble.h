#include <esp_event.h>



ESP_EVENT_DECLARE_BASE(C_BLE_EVENT_BASE);

enum{
    C_BLE_EVENT_ATTENDANCE,
    C_BLE_EVENT_CONNECTED,
};

void ble_init();
#include <stdio.h>

#include "init.h"

static const char* TAG = "main";

void app_main(void){
    setup();
    while (1) vTaskDelay(1000 / portTICK_PERIOD_MS);
}
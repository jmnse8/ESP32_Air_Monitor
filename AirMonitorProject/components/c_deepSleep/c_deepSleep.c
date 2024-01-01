#include <stdio.h>
#include "c_deepSleep.h"

#define START_HOUR  CONFIG_START_HOUR*3600 + CONFIG_START_MINUTE*60

//#define START_MINUTE CONFIG_START_MINUTE

#define FINAL_HOUR  CONFIG_FINAL_HOUR*3600 + CONFIG_FINAL_MINUTE*60

time_t now;
struct tm *nowTime;

time_t momento_final;
struct tm *tiempo_final;


//int segundos_finales= FINAL_HOUR*3600 + FINAL_MINUTE*60;
//int segundos_iniciales= HORA_INICIO*3600 + MINUTOS_INICIO*60;

void init_deep_sleep(void)
{
    time(&now); // Obtener el tiempo actual
    nowTime = localtime(&now);
    
    int ACTUAL_HOUR = nowTime->tm_hour*3600 + nowTime->tm_min*60 + nowTime->tm_sec;

    if(START_HOUR < FINAL_HOUR) {//Caso normal
        if(ACTUAL_HOUR > START_HOUR && ACTUAL_HOUR < FINAL_HOUR){// toca estar despierto
            configure_awake(86400 - ACTUAL_HOUR, 1);
        }
        else{// toca dormir
            configure_sleep(12)
        }
    }
    else {
        if(ACTUAL_HOUR < START_HOUR && ACTUAL_HOUR > FINAL_HOUR){// toca estar despierto
            
        }
        else{// toca dormir

        }
    }

    
}

static void deep_sleep_timer_callback(void* arg)
{
    printf("Entering deep sleep\n");
    int secondsSleep = (int) arg;
    //ESP_ERROR_CHECK(register_timer_wakeup(secondsSleep));
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(secondsSleep));
    // enter deep sleep
    esp_deep_sleep_start();
}

void configure_awake(int secondsAwake, int secondsSleep){
    const esp_timer_create_args_t timer_args = {//mirar psasr parametros a callback
            .callback = &deep_sleep_timer_callback,
            .arg = (void*) secondsSleep,
            .name = "deep_sleep_timer"
    };

    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &deep_sleep_timer));
    ESP_ERROR_CHECK(esp_timer_start_once(deep_sleep_timer, secondsAwake));
}

void configure_sleep(int secondsSleep) {
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(secondsSleep));
    // enter deep sleep
    esp_deep_sleep_start();
}

void check_boot_reason(){

    //esp_log_level_set("*", ESP_LOG_INFO);

    esp_reset_reason_t reset_reason = esp_reset_reason();
    int N = 30;
    char reason_str[N];

    switch (reset_reason) {
        case ESP_RST_POWERON:
            ESP_LOGI(TAG, "Power-on reset");
            strncpy(reason_str, "Power-on reset", N-1);
            break;
        case ESP_RST_EXT:
            ESP_LOGI(TAG, "External reset");
            strncpy(reason_str, "External reset", N-1);
            break;
        case ESP_RST_PANIC:
            ESP_LOGI(TAG, "Exception reset");
            strncpy(reason_str, "Exception reset", N-1);
            break;
        case ESP_RST_DEEPSLEEP:
            ESP_LOGI(TAG, "Deep sleep reset");
            strncpy(reason_str, "Deep sleep reset", N-1);
            break;
        default:
            ESP_LOGI(TAG, "Unknown reset reason");
            strncpy(reason_str, "Unknown reset reason", N-1);
            break;
    }

    write_nvs("wakeup_reason", reason_str);
}
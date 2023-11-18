#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "sdkconfig.h"
#include "esp_event.h"

#include "sensorSGP30.h"
#include "SGP30.h"
#include "I2C.h"


ESP_EVENT_DEFINE_BASE(SENSOR_EVENT_BASE);


#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)

#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA               /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUMBER(CONFIG_I2C_MASTER_PORT_NUM) /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY        /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */

i2c_port_t i2c_num = I2C_MASTER_NUM;

static void sensor_timer_callback_sgp30(void* arg);

static const char* TAG = "SENSOR_SGP30";
static int SENSOR_MODE;
static int SENSOR_FREQ = CONFIG_SENSOR_FREQ;

sgp30_dev_t main_sgp30_sensor;

static esp_timer_handle_t sensor_timer;

void configure_timer_sgp30(){
    const esp_timer_create_args_t sensor_timer_args = {
            .callback = &sensor_timer_callback_sgp30,
            .name = "sensor_timer"
    };

    ESP_ERROR_CHECK(esp_timer_create(&sensor_timer_args, &sensor_timer));

    ESP_ERROR_CHECK(esp_timer_start_periodic(sensor_timer, SENSOR_FREQ*1000000));
}


int change_sample_period_sgp30(int sec){
    if(sec>0){
        ESP_ERROR_CHECK(esp_timer_stop(sensor_timer));
        ESP_ERROR_CHECK(esp_timer_start_periodic(sensor_timer, sec*1000000));
        return ESP_OK;
    }
    return ESP_ERR_INVALID_ARG;
}


void stop_sensor_sgp30(void){
    ESP_ERROR_CHECK(esp_timer_stop(sensor_timer));
    ESP_LOGI(TAG, "Stopped sendor SGP30");
}

void start_sensor_sgp30(void){
    ESP_ERROR_CHECK(esp_timer_start_periodic(sensor_timer, SENSOR_FREQ*1000000));
}


void init_sensor_sgp30(void){

    i2c_master_driver_initialize(i2c_num, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, I2C_MASTER_FREQ_HZ, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE);


    sgp30_init(&main_sgp30_sensor, (sgp30_read_fptr_t)readResponseBytes, (sgp30_write_fptr_t)writeCommandBytes, i2c_num);

    // SGP30 needs to be read every 1s and sends TVOC = 400 14 times when initializing
    for (int i = 0; i < 14; i++) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        sgp30_IAQ_measure(&main_sgp30_sensor);
        ESP_LOGI(TAG, "SGP30 Calibrating... TVOC: %d,  eCO2: %d",  main_sgp30_sensor.TVOC, main_sgp30_sensor.eCO2);
    }

    // Read initial baselines 
    uint16_t eco2_baseline, tvoc_baseline;
    sgp30_get_IAQ_baseline(&main_sgp30_sensor, &eco2_baseline, &tvoc_baseline);
    ESP_LOGI(TAG, "BASELINES - TVOC: %d,  eCO2: %d",  tvoc_baseline, eco2_baseline);


    configure_timer_sgp30();

}


void sensor_timer_callback_sgp30(void* arg)
{

    sgp30_IAQ_measure(&main_sgp30_sensor);
    ESP_LOGI(TAG, "TVOC: %d,  eCO2: %d",  main_sgp30_sensor.TVOC, main_sgp30_sensor.eCO2);
    /* if(SENSOR_MODE==SENSOR_TEMP_MODE || SENSOR_MODE==SENSOR_ALL_MODE){
        float temp;
        readTemperature(0, &temp);
        esp_event_post(SENSOR_EVENT_BASE, SENSOR_TEMP_DATA, &temp, sizeof(temp), 0);
    }

    if(SENSOR_MODE==SENSOR_HUM_MODE || SENSOR_MODE==SENSOR_ALL_MODE){
        float hum;
        readHumidity(0, &hum);
        esp_event_post(SENSOR_EVENT_BASE, SENSOR_HUM_DATA, &hum, sizeof(hum), 0);
    } */
    
}



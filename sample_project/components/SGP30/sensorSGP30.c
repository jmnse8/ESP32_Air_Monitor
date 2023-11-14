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


#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA               /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUMBER(CONFIG_I2C_MASTER_PORT_NUM) /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY        /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */

i2c_port_t i2c_num = I2C_MASTER_NUM;

static void sensor_timer_callback(void* arg);

static const char* TAG = "SENSOR_SGP30";
static int SENSOR_MODE;
static int SENSOR_FREQ;// = CONFIG_SENSOR_FREQ;

sgp30_dev_t main_sgp30_sensor;

static esp_timer_handle_t sensor_timer;

void configure_timer(){
    const esp_timer_create_args_t sensor_timer_args = {
            .callback = &sensor_timer_callback,
            .name = "sensor_timer"
    };

    ESP_ERROR_CHECK(esp_timer_create(&sensor_timer_args, &sensor_timer));

    ESP_ERROR_CHECK(esp_timer_start_periodic(sensor_timer, SENSOR_FREQ*1000000));
}


int change_sample_period(int sec){
    if(sec>0){
        ESP_ERROR_CHECK(esp_timer_stop(sensor_timer));
        ESP_ERROR_CHECK(esp_timer_start_periodic(sensor_timer, sec*1000000));
        return ESP_OK;
    }
    return ESP_ERR_INVALID_ARG;
}


void stop_sensor(void){
    ESP_ERROR_CHECK(esp_timer_stop(sensor_timer));
    ESP_LOGI(TAG, "Stopped sendor SGP30");
}

void start_sensor(void){
    ESP_ERROR_CHECK(esp_timer_start_periodic(sensor_timer, SENSOR_FREQ*1000000));
}


void init_sensor(void){

    i2c_master_driver_initialize(I2C_MASTER_SCL_IO, I2C_MASTER_SDA_IO, I2C_MASTER_NUM, I2C_MASTER_FREQ_HZ, I2C_MASTER_TX_BUF_DISABLE);


    // TODO:leer 14 veces
    configure_timer();

}


void sensor_timer_callback(void* arg)
{
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



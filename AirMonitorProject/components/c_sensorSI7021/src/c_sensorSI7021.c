//_si7021
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "sdkconfig.h"
#include "esp_event.h"


#include "i2c_config.h"
//#include "i2c_config.h"
#include "si7021.h"
#include "c_sensorSI7021.h"


ESP_EVENT_DEFINE_BASE(SENSORSI7021_EVENT_BASE);

//#define _I2C_NUMBER(num) I2C_NUM_##num
//#define I2C_NUMBER(num) _I2C_NUMBER(num)

//#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL               /*!< gpio number for I2C master clock */
//#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA               /*!< gpio number for I2C master data  */
//#define I2C_MASTER_NUM I2C_NUMBER(CONFIG_I2C_MASTER_PORT_NUM) /*!< I2C port number for master dev */
//#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY        /*!< I2C master clock frequency */
//#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
//#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */

//i2c_port_t i2c_num = I2C_MASTER_NUM;

static void sensor_timer_callback_si7021(void* arg);

static const char* TAG = "SENSOR";
static int SENSOR_MODE;
static int SENSOR_FREQ = CONFIG_SENSOR_FREQ;


static esp_timer_handle_t sensor_timer;

void set_sensor_mode_si7021(int m){
    if(m==SENSORSI7021_ALL_MODE
            || m==SENSORSI7021_HUM_MODE
            || m==SENSORSI7021_TEMP_MODE
            || m==SENSORSI7021_DISABLED_MODE){
        SENSOR_MODE = m;
    }
}

void configure_timer_si7021(){
    const esp_timer_create_args_t sensor_timer_args = {
            .callback = &sensor_timer_callback_si7021,
            .name = "sensor_timer"
    };

    ESP_ERROR_CHECK(esp_timer_create(&sensor_timer_args, &sensor_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(sensor_timer, SENSOR_FREQ*1000000));
}


int change_sample_period_si7021(int sec){
    if(sec>0){
        ESP_ERROR_CHECK(esp_timer_stop(sensor_timer));
        ESP_ERROR_CHECK(esp_timer_start_periodic(sensor_timer, sec*1000000));
        return ESP_OK;
    }
    return ESP_ERR_INVALID_ARG;
}


void stop_sensor_si7021(void){
    ESP_ERROR_CHECK(esp_timer_stop(sensor_timer));
    //ESP_ERROR_CHECK(esp_timer_delete(sensor_timer));
    ESP_LOGI(TAG, "Stopped timers");
}

void start_sensor_si7021(void){
    ESP_ERROR_CHECK(esp_timer_start_periodic(sensor_timer, SENSOR_FREQ*1000000));
}


void init_sensor_si7021(void){
    i2c_master_init();
    //ESP_LOGI(TAG, "TEMP: %i",  I2C_MASTER_FREQ_HZ);
       
    //i2c_master_driver_initialize(i2c_num, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, I2C_MASTER_FREQ_HZ, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE);
    SENSOR_MODE = SENSORSI7021_ALL_MODE;
    configure_timer_si7021();
}


void sensor_timer_callback_si7021(void* arg)
{
    if(SENSOR_MODE==SENSORSI7021_TEMP_MODE || SENSOR_MODE==SENSORSI7021_ALL_MODE){
        float temp;
        readTemperature(0, &temp);    
        esp_event_post(SENSORSI7021_EVENT_BASE, SENSORSI7021_TEMP_DATA, &temp, sizeof(temp), 0);
    }

    if(SENSOR_MODE==SENSORSI7021_HUM_MODE || SENSOR_MODE==SENSORSI7021_ALL_MODE){
        float hum;
        readHumidity(0, &hum);    
        esp_event_post(SENSORSI7021_EVENT_BASE, SENSORSI7021_HUM_DATA, &hum, sizeof(hum), 0);
    }
    
}



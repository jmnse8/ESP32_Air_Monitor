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
static int SENSOR_MODE = SENSORSI7021_TEMP_MODE;
static int SENSOR_FREQ = CONFIG_SENSOR_FREQ;


static esp_timer_handle_t tmp_sensor_timer;
static esp_timer_handle_t hum_sensor_timer;

void _refresh_mode(){
    int mode = SENSORSI7021_DISABLED_MODE;

    if(esp_timer_is_active(tmp_sensor_timer))
        mode += SENSORSI7021_TEMP_MODE;
    if (esp_timer_is_active(hum_sensor_timer))
        mode += SENSORSI7021_HUM_MODE;

    SENSOR_MODE = mode;
}


void _si7021_set_sensor_onoff(esp_timer_handle_t* timer, int status){

    if((int)esp_timer_is_active(*timer) != status){
        if(status)
            ESP_ERROR_CHECK(esp_timer_start_periodic(*timer, SENSOR_FREQ * 1000 * 1000));
        else
            ESP_ERROR_CHECK(esp_timer_stop(*timer));

        _refresh_mode();
    }
}

void si7021_set_sensor_onoff(int sensor, int status){
    switch(sensor){
        case SENSORSI7021_TEMP_SENSOR:
            _si7021_set_sensor_onoff(&tmp_sensor_timer, status);
        break;
        case SENSORSI7021_HUM_SENSOR:
            _si7021_set_sensor_onoff(&hum_sensor_timer, status);
        break;
    }
}

void set_sensor_mode_si7021(int m){
    if(m==SENSORSI7021_ALL_MODE
            || m==SENSORSI7021_HUM_MODE
            || m==SENSORSI7021_TEMP_MODE
            || m==SENSORSI7021_DISABLED_MODE){
        SENSOR_MODE = m;
    }
}

int get_sensor_mode_si7021(){
    return SENSOR_MODE;
}

void _configure_timer(esp_timer_handle_t* timer, char *name){

    const esp_timer_create_args_t sensor_timer_args = {
            .callback = &sensor_timer_callback_si7021,
            .name = name
    };

    ESP_ERROR_CHECK(esp_timer_create(&sensor_timer_args, timer));
}


int change_sample_period_si7021(int sec){
    if(sec>0){
        SENSOR_FREQ = sec;
        stop_sensor_si7021();
        start_sensor_si7021();

        return ESP_OK;
    }
    return ESP_ERR_INVALID_ARG;
}


void stop_sensor_si7021(void){
    ESP_ERROR_CHECK(esp_timer_stop(tmp_sensor_timer));
    ESP_ERROR_CHECK(esp_timer_stop(hum_sensor_timer));

    ESP_LOGI(TAG, "Stopped timers");
}

void start_sensor_si7021(void){
    ESP_ERROR_CHECK(esp_timer_start_periodic(tmp_sensor_timer, SENSOR_FREQ * 1000 * 1000));
    ESP_ERROR_CHECK(esp_timer_start_periodic(hum_sensor_timer, SENSOR_FREQ * 1000 * 1000));

    ESP_LOGI(TAG, "Started timers");

}


void init_sensor_si7021(void){
    i2c_master_init();
    //ESP_LOGI(TAG, "TEMP: %i",  I2C_MASTER_FREQ_HZ);
       
    //i2c_master_driver_initialize(i2c_num, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, I2C_MASTER_FREQ_HZ, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE);
    _configure_timer(&tmp_sensor_timer, "tmp_sensor_timer");
    _configure_timer(&hum_sensor_timer, "hum_sensor_timer");

    _si7021_set_sensor_onoff(&tmp_sensor_timer, 1);
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



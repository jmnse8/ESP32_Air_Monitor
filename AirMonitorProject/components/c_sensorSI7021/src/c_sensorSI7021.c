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
#include "si7021.h"
#include "c_sensorSI7021.h"


ESP_EVENT_DEFINE_BASE(SENSORSI7021_EVENT_BASE);

enum{
    SENSORSI7021_DISABLED_MODE = 0,
    SENSORSI7021_TEMP_MODE = 1,
    SENSORSI7021_HUM_MODE = 2,
    SENSORSI7021_ALL_MODE = 3,
};

//#define _I2C_NUMBER(num) I2C_NUM_##num
//#define I2C_NUMBER(num) _I2C_NUMBER(num)

//#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL               /*!< gpio number for I2C master clock */
//#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA               /*!< gpio number for I2C master data  */
//#define I2C_MASTER_NUM I2C_NUMBER(CONFIG_I2C_MASTER_PORT_NUM) /*!< I2C port number for master dev */
//#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY        /*!< I2C master clock frequency */
//#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
//#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */

//i2c_port_t i2c_num = I2C_MASTER_NUM;

//static void _sensor_timer_callback(void* arg);

static const char* TAG = "SENSOR";
static int SENSOR_MODE = SENSORSI7021_TEMP_MODE;

static int SAMPLING_SENSOR_FREQ_TEMP = CONFIG_SENSOR_FREQ;
static int SAMPLING_SENSOR_FREQ_HUM = CONFIG_SENSOR_FREQ;
static int SENSOR_N_PARAMS = 2;


static esp_timer_handle_t temp_sensor_timer;
static esp_timer_handle_t hum_sensor_timer;

// Define el tipo de puntero a función
typedef void (*TimerCallbackFunc)(void*);

// Estructura para almacenar el puntero a función y el nombre del temporizador
typedef struct {
    TimerCallbackFunc callback;
    char* name;
} TimerConfig;

static void _refresh_mode(){
    int mode = SENSORSI7021_DISABLED_MODE;

    if(esp_timer_is_active(tmp_sensor_timer))
        mode += SENSORSI7021_TEMP_MODE;
    if (esp_timer_is_active(hum_sensor_timer))
        mode += SENSORSI7021_HUM_MODE;

    SENSOR_MODE = mode;
}


static void _set_sensor_onoff(esp_timer_handle_t* timer, int status, int samplingSensorFreq){

    if((int)esp_timer_is_active(*timer) != status){
        if(status)
            ESP_ERROR_CHECK(esp_timer_start_periodic(*timer, samplingSensorFreq * 1000 * 1000));
        else
            ESP_ERROR_CHECK(esp_timer_stop(*timer));

        _refresh_mode();
    }
}

void si7021_set_sensor_onoff(int sensor, int status){
    switch(sensor){
        case SENSORSI7021_TEMP_SENSOR:
            _set_sensor_onoff(&tmp_sensor_timer, status);
        break;
        case SENSORSI7021_HUM_SENSOR:
            _set_sensor_onoff(&hum_sensor_timer, status);
        break;
    }
}

char* si7021_get_mode(){
    char* status = (char*)malloc(SENSOR_N_PARAMS * sizeof(char));

    if(status!=NULL){
        status[0] = esp_timer_is_active(tmp_sensor_timer)? '1':'0';
        status[1] = esp_timer_is_active(hum_sensor_timer)? '1':'0';
        return status;
    }
    return NULL;
}

static void _configure_timer(esp_timer_handle_t* timer, TimerConfig* config) {
    const esp_timer_create_args_t sensor_timer_args = {
        .callback = config->callback,
        .name = config->name
    };

    ESP_ERROR_CHECK(esp_timer_create(&sensor_timer_args, timer));
}


int si7021_change_sample_period(int sec){
    if(sec>0){
        SENSOR_FREQ = sec;
        si7021_stop_sensor();
        si7021_start_sensor();

        return ESP_OK;
    }
    return ESP_ERR_INVALID_ARG;
}


void si7021_stop_sensor(void){
    ESP_ERROR_CHECK(esp_timer_stop(tmp_sensor_timer));
    ESP_ERROR_CHECK(esp_timer_stop(hum_sensor_timer));

    ESP_LOGI(TAG, "Stopped timers");
}

void si7021_start_sensor(void){
    ESP_ERROR_CHECK(esp_timer_start_periodic(tmp_sensor_timer, SENSOR_FREQ * 1000 * 1000));
    ESP_ERROR_CHECK(esp_timer_start_periodic(hum_sensor_timer, SENSOR_FREQ * 1000 * 1000));

    ESP_LOGI(TAG, "Started timers");

}


void si7021_init_sensor(void){
    i2c_master_init();
    
    TimerConfig timerConfigTemp = {
        .callback = &_sensor_timer_callback_temp,
        .name = "temp_timer"
    };

    TimerConfig timerConfigHum = {
        .callback = &_sensor_timer_callback_hum,
        .name = "hum_timer"
    };
    _configure_timer(&temp_sensor_timer, &timerConfigTemp);
    _configure_timer(&hum_sensor_timer, &timerConfigHum);

    _set_sensor_onoff(&temp_sensor_timer, 1, SAMPLING_SENSOR_FREQ_TEMP);
    _set_sensor_onoff(&hum_sensor_timer, 1, SAMPLING_SENSOR_FREQ_HUM);
    
    //_configure_timer(&tmp_sensor_timer, "tmp_sensor_timer");
    //_configure_timer(&hum_sensor_timer, "hum_sensor_timer");

    //_set_sensor_onoff(&tmp_sensor_timer, 1);
}



static void _sensor_timer_callback_temp(void* arg) {
    if(SENSOR_MODE==SENSORSI7021_TEMP_MODE || SENSOR_MODE==SENSORSI7021_ALL_MODE){
        float temp;
        readTemperature(0, &temp);    
        esp_event_post(SENSORSI7021_EVENT_BASE, SENSORSI7021_TEMP_DATA, &temp, sizeof(temp), 0);
        free(temp);
    }
}

static void _sensor_timer_callback_hum(void* arg) {
    if(SENSOR_MODE==SENSORSI7021_HUM_MODE || SENSOR_MODE==SENSORSI7021_ALL_MODE){
        float hum;
        readHumidity(0, &hum);    
        esp_event_post(SENSORSI7021_EVENT_BASE, SENSORSI7021_HUM_DATA, &hum, sizeof(hum), 0);
        free(hum);
    }
}

//_si7021
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
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
static void _send_sensor_timer_callback_temp(void* arg);
static void _send_sensor_timer_callback_hum(void* arg);
static void _sampling_sensor_timer_callback_temp(void* arg);
static void _sampling_sensor_timer_callback_hum(void* arg);

static const char* TAG = "SENSOR";
static int SENSOR_MODE = SENSORSI7021_TEMP_MODE;

static int SAMPLING_SENSOR_FREQ_TEMP = CONFIG_SAMPLING_SENSOR_FREQ_TEMP;
static int SAMPLING_SENSOR_FREQ_HUM = CONFIG_SAMPLING_SENSOR_FREQ_HUM;
static int SEND_SENSOR_FREQ_TEMP = CONFIG_SEND_SENSOR_FREQ_TEMP;
static int SEND_SENSOR_FREQ_HUM = CONFIG_SEND_SENSOR_FREQ_HUM;

static int SENSOR_N_PARAMS = 2;

static esp_timer_handle_t temp_sampling_sensor_timer;
static esp_timer_handle_t hum_sampling_sensor_timer;

static esp_timer_handle_t temp_send_sensor_timer;
static esp_timer_handle_t hum_send_sensor_timer;

// Define el tipo de puntero a función
typedef void (*TimerCallbackFunc)(void*);

// Estructura para almacenar el puntero a función y el nombre del temporizador
typedef struct {
    TimerCallbackFunc callback;
    char* name;
} TimerConfig;

typedef struct {
    float sum;
    int counter;
} SensorData;

static SemaphoreHandle_t tempMutex;
static SensorData tempData;

static SemaphoreHandle_t humMutex;
static SensorData humData;

static void _refresh_mode(){
    int mode = SENSORSI7021_DISABLED_MODE;

    if(esp_timer_is_active(temp_send_sensor_timer))
        mode += SENSORSI7021_TEMP_MODE;
    if (esp_timer_is_active(hum_send_sensor_timer))
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
            _set_sensor_onoff(&temp_sampling_sensor_timer, status, SAMPLING_SENSOR_FREQ_TEMP);
            _set_sensor_onoff(&temp_send_sensor_timer, status, SEND_SENSOR_FREQ_TEMP);


        break;
        case SENSORSI7021_HUM_SENSOR:
            _set_sensor_onoff(&hum_sampling_sensor_timer, status, SAMPLING_SENSOR_FREQ_HUM);
            _set_sensor_onoff(&hum_send_sensor_timer, status, SEND_SENSOR_FREQ_HUM);
        break;
    } 
}

char* si7021_get_mode(){
    char* status = (char*)malloc(SENSOR_N_PARAMS * sizeof(char));

    if(status!=NULL){
        status[0] = esp_timer_is_active(temp_send_sensor_timer)? '1':'0';
        status[1] = esp_timer_is_active(hum_send_sensor_timer)? '1':'0';
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


void si7021_init_sensor(void){
    SENSOR_MODE = SENSORSI7021_ALL_MODE;
    i2c_master_init();

    tempMutex = xSemaphoreCreateMutex();
    humMutex = xSemaphoreCreateMutex();

    tempData.counter = 0;
    tempData.sum = 0;
    
    humData.counter = 0;
    humData.sum = 0;

    TimerConfig timerSamplingConfigTemp = {
        .callback = &_sampling_sensor_timer_callback_temp,
        .name = "temp_sampling_timer"
    };

    TimerConfig timerSamplingConfigHum = {
        .callback = &_sampling_sensor_timer_callback_hum,
        .name = "hum_sampling_timer"
    };

    TimerConfig timerSendConfigTemp = {
        .callback = &_send_sensor_timer_callback_temp,
        .name = "temp_send_timer"
    };

    TimerConfig timerSendConfigHum = {
        .callback = &_send_sensor_timer_callback_hum,
        .name = "hum_send_timer"
    };

    _configure_timer(&temp_sampling_sensor_timer, &timerSamplingConfigTemp);
    _configure_timer(&hum_sampling_sensor_timer, &timerSamplingConfigHum);
    
    _configure_timer(&temp_send_sensor_timer, &timerSendConfigTemp);
    _configure_timer(&hum_send_sensor_timer, &timerSendConfigHum);

    _set_sensor_onoff(&temp_sampling_sensor_timer, 1, SAMPLING_SENSOR_FREQ_TEMP);
    _set_sensor_onoff(&hum_sampling_sensor_timer, 1, SAMPLING_SENSOR_FREQ_HUM);

    _set_sensor_onoff(&temp_send_sensor_timer, 1, SEND_SENSOR_FREQ_TEMP);
    _set_sensor_onoff(&hum_send_sensor_timer, 1, SEND_SENSOR_FREQ_HUM);
}



static void _send_sensor_timer_callback_temp(void* arg) {
    if(SENSOR_MODE==SENSORSI7021_TEMP_MODE || SENSOR_MODE==SENSORSI7021_ALL_MODE){
        float temp;

        xSemaphoreTake(tempMutex, portMAX_DELAY);
        temp = tempData.sum / tempData.counter;
        tempData.sum = 0;
        tempData.counter = 0;
        xSemaphoreGive(tempMutex);

        esp_event_post(SENSORSI7021_EVENT_BASE, SENSORSI7021_TEMP_DATA, &temp, sizeof(temp), 0);
    }
}

static void _send_sensor_timer_callback_hum(void* arg) {
    if(SENSOR_MODE==SENSORSI7021_HUM_MODE || SENSOR_MODE==SENSORSI7021_ALL_MODE){
        float hum;

        xSemaphoreTake(humMutex, portMAX_DELAY);
        hum = humData.sum / humData.counter;
        humData.sum = 0;
        humData.counter = 0;
        xSemaphoreGive(humMutex);

        esp_event_post(SENSORSI7021_EVENT_BASE, SENSORSI7021_HUM_DATA, &hum, sizeof(hum), 0);
    }
}

static void _sampling_sensor_timer_callback_temp(void* arg) {
    if(SENSOR_MODE==SENSORSI7021_TEMP_MODE || SENSOR_MODE==SENSORSI7021_ALL_MODE){
        float temp;
        readTemperature(0, &temp); 

        xSemaphoreTake(tempMutex, portMAX_DELAY);
        tempData.sum += temp;
        tempData.counter++;
        xSemaphoreGive(tempMutex);
    }
}

static void _sampling_sensor_timer_callback_hum(void* arg) {
    if(SENSOR_MODE==SENSORSI7021_HUM_MODE || SENSOR_MODE==SENSORSI7021_ALL_MODE){
        float hum;
        readHumidity(0, &hum);    
        
        xSemaphoreTake(humMutex, portMAX_DELAY);
        humData.sum += hum;
        humData.counter++;
        xSemaphoreGive(humMutex);
    }
}

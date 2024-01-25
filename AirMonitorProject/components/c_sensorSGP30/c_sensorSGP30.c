#include <stdio.h>
#include "c_sensorSGP30.h"


ESP_EVENT_DEFINE_BASE(SENSORSGP30_EVENT_BASE);


#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)

#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA               /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUMBER(CONFIG_I2C_MASTER_PORT_NUM) /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY        /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */

enum{
    SENSORSGP30_DISABLED_MODE = 0,
    SENSORSGP30_TVOC_MODE = 1,
    SENSORSGP30_ECO2_MODE = 2,
    SENSORSGP30_ALL_MODE = 3,
};

i2c_port_t i2c_num = I2C_MASTER_NUM;

static void _sensor_send_timer_callback_tvoc(void* arg);
static void _sensor_send_timer_callback_eco2(void* arg);
static void _sampling_sensor_timer_callback(void* arg);

static const char* TAG = "SENSOR_SGP30";
static int SENSOR_MODE;
static int SEND_SENSOR_FREQ_TVOC = CONFIG_SEND_SENSOR_FREQ_TVOC;
static int SEND_SENSOR_FREQ_ECO2 = CONFIG_SEND_SENSOR_FREQ_ECO2;
static int SAMPLING_SENSOR_FREQ = CONFIG_SAMPLING_SENSOR_FREQ;
static int SENSOR_N_PARAMS = 2;


sgp30_dev_t main_sgp30_sensor;

static esp_timer_handle_t send_tvoc_sensor_timer;
static esp_timer_handle_t send_eco2_sensor_timer;
static esp_timer_handle_t sampling_sensor_timer;

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

static SemaphoreHandle_t tvocMutex;
static SensorData tvocData;

static SemaphoreHandle_t eco2Mutex;
static SensorData eco2Data;


static void _refresh_mode(){ 
    int mode = SENSORSGP30_DISABLED_MODE;

    if(esp_timer_is_active(send_eco2_sensor_timer))
        mode += SENSORSGP30_ECO2_MODE;
    if (esp_timer_is_active(send_tvoc_sensor_timer))
        mode += SENSORSGP30_TVOC_MODE;

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

char* sgp30_get_mode(){
    char* status = (char*)malloc(SENSOR_N_PARAMS * sizeof(char));

    if(status!=NULL){
        status[0] = esp_timer_is_active(send_eco2_sensor_timer)? '1':'0';
        status[1] = esp_timer_is_active(send_tvoc_sensor_timer)? '1':'0';
        return status;
    }
    return NULL;
}

void sgp30_set_sensor_onoff(int sensor, int status){
    switch(sensor){
        case SENSORSGP30_ECO2_SENSOR:
            _set_sensor_onoff(&send_eco2_sensor_timer, status, SEND_SENSOR_FREQ_ECO2);
        break;
        case SENSORSGP30_TVOC_SENSOR:
            _set_sensor_onoff(&send_tvoc_sensor_timer, status, SEND_SENSOR_FREQ_TVOC);
        break;
    } 
}


static void _configure_timer(esp_timer_handle_t* timer, TimerConfig* config) {
    const esp_timer_create_args_t sensor_timer_args = {
        .callback = config->callback,
        .name = config->name
    };

    ESP_ERROR_CHECK(esp_timer_create(&sensor_timer_args, timer));
}


void sgp30_init_sensor(void) {
    SENSOR_MODE = SENSORSGP30_ALL_MODE;

    i2c_master_driver_initialize(i2c_num, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, I2C_MASTER_FREQ_HZ, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE);

    sgp30_init(&main_sgp30_sensor, (sgp30_read_fptr_t)readResponseBytes, (sgp30_write_fptr_t)writeCommandBytes, i2c_num);

    // SGP30 needs to be read every 1s and sends TVOC = 400 14 times when initializing
    for (int i = 0; i < 14; i++) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        sgp30_IAQ_measure(&main_sgp30_sensor);
        //ESP_LOGI(TAG, "SGP30 Calibrating... TVOC: %d,  eCO2: %d",  main_sgp30_sensor.TVOC, main_sgp30_sensor.eCO2);
    }

    // Read initial baselines 
    uint16_t eco2_baseline, tvoc_baseline;
    sgp30_get_IAQ_baseline(&main_sgp30_sensor, &eco2_baseline, &tvoc_baseline);
    ESP_LOGI(TAG, "BASELINES - TVOC: %d,  eCO2: %d",  tvoc_baseline, eco2_baseline);

    eco2Mutex = xSemaphoreCreateMutex();
    tvocMutex = xSemaphoreCreateMutex();

    eco2Data.counter = 0;
    eco2Data.sum = 0;
    
    tvocData.counter = 0;
    tvocData.sum = 0;

    TimerConfig timerSamplingConfigTemp = {
        .callback = &_sampling_sensor_timer_callback,
        .name = "sampling_timer"
    };

    TimerConfig timerConfigEco2 = {
        .callback = &_sensor_send_timer_callback_eco2,
        .name = "eco2_send_timer"
    };

    TimerConfig timerConfigTvoc = {
        .callback = &_sensor_send_timer_callback_tvoc,
        .name = "tvoc_send_timer"
    };

    _configure_timer(&send_eco2_sensor_timer, &timerConfigEco2);
    _configure_timer(&send_tvoc_sensor_timer, &timerConfigTvoc);
    _configure_timer(&sampling_sensor_timer, &timerSamplingConfigTemp);

    _set_sensor_onoff(&send_tvoc_sensor_timer, 1, SEND_SENSOR_FREQ_TVOC);
    _set_sensor_onoff(&send_eco2_sensor_timer, 1, SEND_SENSOR_FREQ_ECO2);
    _set_sensor_onoff(&sampling_sensor_timer, 1, SAMPLING_SENSOR_FREQ);

}


static void _sensor_send_timer_callback_tvoc(void* arg) {
    if(SENSOR_MODE==SENSORSGP30_TVOC_MODE || SENSOR_MODE==SENSORSGP30_ALL_MODE){
        uint16_t tvoc;

        xSemaphoreTake(tvocMutex, portMAX_DELAY);
        tvoc = tvocData.sum / tvocData.counter;
        tvocData.sum = 0;
        tvocData.counter = 0;
        xSemaphoreGive(tvocMutex);

        esp_event_post(SENSORSGP30_EVENT_BASE, SENSORSGP30_TVOC_DATA, &tvoc, sizeof(tvoc), 0);
    }
}

static void _sensor_send_timer_callback_eco2(void* arg) {
    if(SENSOR_MODE==SENSORSGP30_ECO2_MODE || SENSOR_MODE==SENSORSGP30_ALL_MODE){
        uint16_t eco2;

        xSemaphoreTake(eco2Mutex, portMAX_DELAY);
        eco2 = eco2Data.sum / eco2Data.counter;
        eco2Data.sum = 0;
        eco2Data.counter = 0;
        xSemaphoreGive(eco2Mutex);

        esp_event_post(SENSORSGP30_EVENT_BASE, SENSORSGP30_ECO2_DATA, &eco2, sizeof(eco2), 0);
    }
    xSemaphoreGive(tvocMutex);
}

static void _sampling_sensor_timer_callback(void* arg) {
    sgp30_IAQ_measure(&main_sgp30_sensor);

    xSemaphoreTake(eco2Mutex, portMAX_DELAY);
    if(eco2Data.counter < 100) {
        eco2Data.sum += main_sgp30_sensor.eCO2;
        eco2Data.counter++;
    }
    xSemaphoreGive(eco2Mutex);

    xSemaphoreTake(tvocMutex, portMAX_DELAY);
    if(tvocData.counter < 100) {
        tvocData.sum += main_sgp30_sensor.TVOC;
        tvocData.counter++;
    }
    xSemaphoreGive(tvocMutex);
}

void sgp30_tvoc_change_send_freq(int sec) {
    if(esp_timer_is_active(send_tvoc_sensor_timer))
        if (sec > SAMPLING_SENSOR_FREQ) {
            SEND_SENSOR_FREQ_TVOC = sec;
            ESP_ERROR_CHECK(esp_timer_stop(send_tvoc_sensor_timer));
            ESP_ERROR_CHECK(esp_timer_start_periodic(send_tvoc_sensor_timer, SEND_SENSOR_FREQ_TVOC * 1000 * 1000));
        }
}

void sgp30_eco2_change_send_freq(int sec) {
    if(esp_timer_is_active(send_eco2_sensor_timer))
        if (sec > SAMPLING_SENSOR_FREQ) {
            SEND_SENSOR_FREQ_ECO2 = sec;
            ESP_ERROR_CHECK(esp_timer_stop(send_eco2_sensor_timer));
            ESP_ERROR_CHECK(esp_timer_start_periodic(send_eco2_sensor_timer, SEND_SENSOR_FREQ_ECO2 * 1000 * 1000));
        }
}

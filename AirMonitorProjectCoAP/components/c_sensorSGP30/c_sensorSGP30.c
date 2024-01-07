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

static void _sensor_timer_callback(void* arg);

static const char* TAG = "SENSOR_SGP30";
static int SENSOR_MODE;
static int SENSOR_FREQ = CONFIG_SENSOR_FREQ;
static int SENSOR_N_PARAMS = 2;


sgp30_dev_t main_sgp30_sensor;

static esp_timer_handle_t tvoc_sensor_timer;
static esp_timer_handle_t eco2_sensor_timer;


static void _refresh_mode(){
    int mode = SENSORSGP30_DISABLED_MODE;

    if(esp_timer_is_active(eco2_sensor_timer))
        mode += SENSORSGP30_ECO2_MODE;
    if (esp_timer_is_active(tvoc_sensor_timer))
        mode += SENSORSGP30_TVOC_MODE;

    SENSOR_MODE = mode;
}


static void _set_sensor_onoff(esp_timer_handle_t* timer, int status){

    if((int)esp_timer_is_active(*timer) != status){
        if(status)
            ESP_ERROR_CHECK(esp_timer_start_periodic(*timer, SENSOR_FREQ * 1000 * 1000));
        else
            ESP_ERROR_CHECK(esp_timer_stop(*timer));

        _refresh_mode();
    }
}

char* sgp30_get_mode(){
    char* status = (char*)malloc(SENSOR_N_PARAMS * sizeof(char));

    if(status!=NULL){
        status[0] = esp_timer_is_active(eco2_sensor_timer)? '1':'0';
        status[1] = esp_timer_is_active(tvoc_sensor_timer)? '1':'0';
        return status;
    }
    return NULL;    
}

void sgp30_set_sensor_onoff(int sensor, int status){
    switch(sensor){
        case SENSORSGP30_ECO2_SENSOR:
            _set_sensor_onoff(&eco2_sensor_timer, status);
        break;
        case SENSORSGP30_TVOC_SENSOR:
            _set_sensor_onoff(&tvoc_sensor_timer, status);
        break;
    }
}


static void _configure_timer(esp_timer_handle_t* timer, char *name){

    const esp_timer_create_args_t sensor_timer_args = {
            .callback = &_sensor_timer_callback,
            .name = name
    };

    ESP_ERROR_CHECK(esp_timer_create(&sensor_timer_args, timer));
}


void sgp30_stop_sensor(void){
    ESP_ERROR_CHECK(esp_timer_stop(eco2_sensor_timer));
    ESP_ERROR_CHECK(esp_timer_stop(tvoc_sensor_timer));

    ESP_LOGI(TAG, "Stopped timers");
}

void sgp30_start_sensor(void){
    ESP_ERROR_CHECK(esp_timer_start_periodic(eco2_sensor_timer, SENSOR_FREQ * 1000 * 1000));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tvoc_sensor_timer, SENSOR_FREQ * 1000 * 1000));

    ESP_LOGI(TAG, "Started timers");

}

int sgp30_change_sample_period(int sec){
    if(sec>0){
        SENSOR_FREQ = sec;
        sgp30_stop_sensor();
        sgp30_start_sensor();

        return ESP_OK;
    }
    return ESP_ERR_INVALID_ARG;
}


void sgp30_init_sensor(void) {

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

    _configure_timer(&tvoc_sensor_timer, "tvoc_timer");
    _configure_timer(&eco2_sensor_timer, "eco2_timer");

    _set_sensor_onoff(&tvoc_sensor_timer, 1);
    _set_sensor_onoff(&eco2_sensor_timer, 1);
}


static void _sensor_timer_callback(void* arg) {
    sgp30_IAQ_measure(&main_sgp30_sensor);

    if(SENSOR_MODE==SENSORSGP30_TVOC_MODE || SENSOR_MODE==SENSORSGP30_ALL_MODE){
        uint16_t tvoc = main_sgp30_sensor.TVOC;
        esp_event_post(SENSORSGP30_EVENT_BASE, SENSORSGP30_TVOC_DATA, &tvoc, sizeof(tvoc), 0);
    }

    if(SENSOR_MODE==SENSORSGP30_ECO2_MODE || SENSOR_MODE==SENSORSGP30_ALL_MODE){
        uint16_t eCO2 = main_sgp30_sensor.eCO2;
        esp_event_post(SENSORSGP30_EVENT_BASE, SENSORSGP30_ECO2_DATA, &eCO2, sizeof(eCO2), 0);
    }
}



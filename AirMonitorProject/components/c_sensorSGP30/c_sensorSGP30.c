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

i2c_port_t i2c_num = I2C_MASTER_NUM;

static void sensor_timer_callback_sgp30(void* arg);

static const char* TAG = "SENSOR_SGP30";
static int SENSOR_MODE;
static int SENSOR_FREQ = CONFIG_SENSOR_FREQ;

sgp30_dev_t main_sgp30_sensor;

static esp_timer_handle_t sensor_timer;

static void configure_timer_sgp30() {
    const esp_timer_create_args_t sensor_timer_args = {
            .callback = &sensor_timer_callback_sgp30,
            .name = "sensor_timer"
    };

    ESP_ERROR_CHECK(esp_timer_create(&sensor_timer_args, &sensor_timer));
    start_sensor_sgp30();
}

void set_sensor_mode(int m){
    if(m==SENSORSGP30_ALL_MODE
            || m==SENSORSGP30_TVOC_MODE
            || m==SENSORSGP30_ECO2_MODE
            || m==SENSORSGP30_DISABLED_MODE){
        SENSOR_MODE = m;
    }
}


int change_sample_period_sgp30(int sec) {
    if(sec>0){
        SENSOR_FREQ = sec;
        ESP_ERROR_CHECK(esp_timer_stop(sensor_timer));
        start_sensor_sgp30();
        ESP_LOGI(TAG, "FREQ changed to %d seconds per sample", sec);
        return ESP_OK;
    }
    return ESP_ERR_INVALID_ARG;
}


void stop_sensor_sgp30(void) {
    ESP_ERROR_CHECK(esp_timer_stop(sensor_timer));
    ESP_LOGI(TAG, "Stopped sensor SGP30");
}

void start_sensor_sgp30(void) {
    ESP_ERROR_CHECK(esp_timer_start_periodic(sensor_timer, SENSOR_FREQ*1000*1000));
    ESP_LOGI(TAG, "Started sensor SGP30");
}


void init_sensor_sgp30(void) {

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

    SENSOR_MODE = SENSORSGP30_ALL_MODE;

    configure_timer_sgp30();

}


static void sensor_timer_callback_sgp30(void* arg) {
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



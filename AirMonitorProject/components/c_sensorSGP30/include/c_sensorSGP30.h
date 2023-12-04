#ifndef __SENSOR_SGP30_H
#define __SENSOR_SGP30_H

#include <string.h>
#include <unistd.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "sdkconfig.h"
#include "esp_event.h"

#include "c_SGP30.h"
#include "c_I2C.h"

ESP_EVENT_DECLARE_BASE(SENSORSGP30_EVENT_BASE);

enum{
    SENSORSGP30_TVOC_MODE,
    SENSORSGP30_ECO2_MODE,
    SENSORSGP30_ALL_MODE,
    SENSORSGP30_DISABLED_MODE
};

enum{
    SENSORSGP30_TVOC_DATA,
    SENSORSGP30_ECO2_DATA
};

/*
    Inicializa el sensor con su timer de muestreo
*/
void init_sensor_sgp30(void);

/*
    Cambia el modo de muestreo del sensor:
        SENSOR_TVOC_MODE = 0
        SENSOR_ECO2_MODE = 1
        SENSOR_ALL_MODE = 2
        SENSOR_DISABLED_MODE = 3
*/
void set_sensor_mode_sgp30(int m);

/*
    Cambia la frecuencia de muestreo en segundos
*/
int change_sample_period_sgp30(int sec);

/*
    Detener el sensor
*/
void stop_sensor_sgp30(void);

/*
    Iniciar el sensor tras ejecutar stop_sensor()
*/
void start_sensor_sgp30(void);

#endif
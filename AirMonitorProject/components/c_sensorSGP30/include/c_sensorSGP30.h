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

enum SGP30_SENSOR_ENUM{
    SENSORSGP30_TVOC_SENSOR = 1,
    SENSORSGP30_ECO2_SENSOR = 2,
};

enum{
    SENSORSGP30_TVOC_DATA,
    SENSORSGP30_ECO2_DATA
};

/*
    Inicializa el sensor con su timer de muestreo
*/
void sgp30_init_sensor(void);

/*
    Cambia la frecuencia de muestreo en segundos
*/
int sgp30_change_sample_period(int sec);


/**
 * @brief Gets the status of sensorized parameters and returns a char array.
 *
 * This function creates a char array where each element represents the status of a sensorized parameter.
 * The value is '1' if the corresponding sensorized parameter is active and '0' otherwise.
 *
 * @return A dynamically allocated char array containing '1' or '0' for each sensorized parameter status.
 *         The caller is responsible for freeing the allocated memory.
 */
char* sgp30_get_mode();

/*
    Detener la sesorización de datos del sgp30
*/
void sgp30_stop_sensor(void);

/*
    Iniciar la sesorización de datos del sgp30
*/
void sgp30_start_sensor(void);

/**
    @brief Detener/Iniciar la sensorización de un dato concreto del sgp30
    @param sensor: Usa el enum SGP30_SENSOR_ENUM. Mira el .h
    @param status: (0 off | 1 on)
*/
void sgp30_set_sensor_onoff(int sensor, int status);

#endif
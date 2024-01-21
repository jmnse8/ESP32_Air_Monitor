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


/**
    @brief Detener/Iniciar la sensorización de un dato concreto del sgp30
    @param sensor: Usa el enum SGP30_SENSOR_ENUM. Mira el .h
    @param status: (0 off | 1 on)
*/
void sgp30_set_sensor_onoff(int sensor, int status);

/**
    @brief Cambia la frecuancia de envío de tvoc del sgp30
    @param sec: segundos de la nueva frecuencia de envío
*/
void sgp30_tvoc_change_send_freq(int sec);

/**
    @brief Cambia la frecuancia de envío de eco2 del sgp30
    @param sec: segundos de la nueva frecuencia de envío
*/
void sgp30_eco2_change_send_freq(int sec);

#endif
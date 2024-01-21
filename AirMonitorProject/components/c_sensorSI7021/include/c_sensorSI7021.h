#ifndef __SENSORSI7021_H
#define __SENSORSI7021_H


ESP_EVENT_DECLARE_BASE(SENSORSI7021_EVENT_BASE);

enum SI7021_SENSOR_ENUM{
    SENSORSI7021_TEMP_SENSOR = 1,
    SENSORSI7021_HUM_SENSOR = 2,
};

enum{
    SENSORSI7021_TEMP_DATA,
    SENSORSI7021_HUM_DATA
};

/*
    Inicializa el sensor con su timer de muestreo
*/
void si7021_init_sensor(void);


/**
 * @brief Gets the status of sensorized parameters and returns a char array.
 *
 * This function creates a char array where each element represents the status of a sensorized parameter.
 * The value is '1' if the corresponding sensorized parameter is active and '0' otherwise.
 *
 * @return A dynamically allocated char array containing '1' or '0' for each sensorized parameter status.
 *         The caller is responsible for freeing the allocated memory.
 */
char* si7021_get_mode();

/**
    @brief Detener/Iniciar la sensorización de un dato concreto del si7021
    @param sensor: Usa el enum SI7021_SENSOR_ENUM. Mira el .h
    @param status: (0 off | 1 on)
*/
void si7021_set_sensor_onoff(int sensor, int status);

/**
    @brief Cambia la frecuancia de envío de temperatura del si7021
    @param sec: segundos de la nueva frecuencia de envío
*/
void si7021_temp_change_send_freq(int sec);

/**
    @brief Cambia la frecuancia de envío de humedad del si7021
    @param sec: segundos de la nueva frecuencia de envío
*/
void si7021_hum_change_send_freq(int sec);

#endif
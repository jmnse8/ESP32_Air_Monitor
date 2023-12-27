#ifndef __SENSORSI7021_H
#define __SENSORSI7021_H


ESP_EVENT_DECLARE_BASE(SENSORSI7021_EVENT_BASE);

enum{
    SENSORSI7021_DISABLED_MODE = 0,
    SENSORSI7021_TEMP_MODE,
    SENSORSI7021_HUM_MODE,
    SENSORSI7021_ALL_MODE,
};

enum{
    SENSORSI7021_TEMP_DATA,
    SENSORSI7021_HUM_DATA
};

/*
    Inicializa el sensor con su timer de muestreo
*/
void init_sensor_si7021(void);

int get_sensor_mode_si7021();

/*
    Cambia el modo de muestreo del sensor:
        SENSOR_TEMP_MODE = 0
        SENSOR_HUM_MODE = 1
        SENSOR_ALL_MODE = 2
        SENSOR_DISABLED_MODE = 3
*/
void set_sensor_mode_si7021(int m);

/*
    Cambia la frecuencia de muestreo en segundos
*/
int change_sample_period_si7021(int sec);

/*
    Detener el sensor
*/
void stop_sensor_si7021(void);

/*
    Iniciar el sensor tras ejecutar stop_sensor()
*/
void start_sensor_si7021(void);

#endif
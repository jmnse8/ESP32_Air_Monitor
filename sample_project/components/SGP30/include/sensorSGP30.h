#ifndef __SENSOR_H
#define __SENSOR_H

/*
    Inicializa el sensor con su timer de muestreo
*/
void init_sensor_sgp30(void);

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
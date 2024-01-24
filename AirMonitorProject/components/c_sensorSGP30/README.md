# Component SGP30

The main code is in c_sensorSGP30.c and c_sensorSGP30.h

Interfaces ESP32 with SGP30 Air Quality Sensor via I²C.

- **TVOC**: Total Volatile Organic Compounds in ppb
- **eCO2**: CO2 in ppm

Component SGP30 based on:

https://github.com/co-env/esp32_SGP30

https://github.com/adafruit/Adafruit_SGP30

The component uses the [component I2C](AirMonitorProject/components/c_I2C/README.md) to comunicate with the sensor.

On the component, there are three timers:
1. **SAMPLING_SENSOR_FREQ**: There is only one timer for sampling because the component reads the two parameters simultaneously. By default it is 2 seconds.

2. **SEND_SENSOR_FREQ_TVOC**: Sensor send rate in seconds for TVOC. This is independent from the samping timer. By default it is 10 seconds.

3. **SEND_SENSOR_FREQ_ECO2**: Sensor send rate in seconds for ECO2. This is independent from the samping timer. By default it is 10 seconds.

The component computes the **mean** of all the samples that have been recovered since the last broadcast. This variable implements a mutex to save simultaneous readings or writes from the timer handlers, and the means are independent for each type.

Also you are able to **turn on/off** each timer for sending the samples in the events or **changing the sending frequency** in seconds only if its bigger from the sampling rate.

The component sends the samplings through two event posts:
1. SENSORSGP30_TVOC_DATA
2. SENSORSGP30_ECO2_DATA
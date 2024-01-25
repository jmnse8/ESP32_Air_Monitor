# Component SI7021

- **Temperature**
- **Humidity**

The main code is in c_sensorSI7021.c and c_sensorSI7021.h

This component doesn't use the [component I2C](AirMonitorProject/components/c_I2C/README.md) to comunicate with the sensor. It has his own I2C interface due to problems, in the future will use the [component I2C](AirMonitorProject/components/c_I2C/README.md)

On the component, there are four timers:
1. **SAMPLING_SENSOR_FREQ_TEMP**: Temperature sensor sampling rate in seconds. By default it is 2 seconds.

2. **SAMPLING_SENSOR_FREQ_HUM**: Humidity sensor sampling rate in seconds. By default it is 2 seconds.

3. **SEND_SENSOR_FREQ_TEMP**: Temperature sensor send rate in seconds. This is independent from the samping timer of temp. By default it is 10 seconds.

4. **SEND_SENSOR_FREQ_HUM**: SHumidity sensor send rate in seconds. This is independent from the samping timer of hum. By default it is 2 seconds.

The component computes the **mean** of all the samples that have been recovered since the last broadcast. This variable implements a mutex to save simultaneous readings or writes from the timer handlers, and the means are independent for each type.

Also you are able to **turn on/off** each timer for sampling and sending the samples in the events or **changing the sending frequency** in seconds only if its bigger from the sampling rate.

The component sends the samplings through two event posts:
1. SENSORSI7021_TEMP_DATA
2. SENSORSI7021_HUM_DATA
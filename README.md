# Proyecto-final
Sistema de monitorización de la calidad del aire con ESP32

### SGP 30

TVOC: Total Volatile Organic Compounds in ppb
eCO2: CO2 in ppm

Componente SGP30 basado en:

https://github.com/co-env/esp32_SGP30

https://github.com/adafruit/Adafruit_SGP30

## Definición de FSM

### Estados

- Sin wifi : 

- provisionando: 

- Provisionado:

### Jerarquía MQTT

EDIFICIO/NUMPLANTA/SALA/SENSOR

- Ejemplo:
    Edificio 5, planta 2, sala 12 y sensor humedad
    5/2/12/HUM


EDIFICIO/NUMPLANTA/SALA/SENSOR/DIS -> desactiva el sensor

EDIFICIO/NUMPLANTA/SALA/SENSOR/ENB -> activa el sensor

EDIFICIO/NUMPLANTA/SALA/SENSOR/FRQ -> actualiza la frecuencia de muestreo

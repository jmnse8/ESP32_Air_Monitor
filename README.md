# Proyecto-final
Sistema de monitorización de la calidad del aire con ESP32


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
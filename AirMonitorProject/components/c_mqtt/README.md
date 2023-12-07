# Component MQTT
Lo he llamado mqtt_com(ponent) para que el nombre coincida con otra librería parecida

## TO USE THIS COMPONENT, ADD IN YOUR MAIN:
```
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h"

ESP_ERROR_CHECK(nvs_flash_init());
ESP_ERROR_CHECK(esp_netif_init());
ESP_ERROR_CHECK(esp_event_loop_create_default());
ESP_ERROR_CHECK(example_connect());
```

### If can't find protocol_examples_common, add in CMakelists.txt(the one at the same level as /main):
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)


## Authentication: username & pwd
[Tutorial](http://www.steves-internet-guide.com/mqtt-username-password-example/)

| USERNAME  |    PWD   |
|-----------|----------|
|   user1   |   user1  |



## Hierarchy
**NUMPLANTA/SALA/SENSOR**

| NUMPLANTA | SALA | SENSOR |
|-----------|------|--------|
|     2     |  3   |  TMP   |


**VALUES**

- NUMPLANTA: int

- SALA: int

- SENSOR: string

    - TMP: Temperatura

    - HUM: Humedad

    - QOA: Calidad de aire


## CONTROL  

### Sensor concreto: 

- Activar/Desactivar:

    - Topic: NUMPLANTA/SALA/SENSOR/***ONOFF***
    
    - Value: 0 Desactivar, 1 Activar

- Cambiar frecuencia de muestreo:

    - Topic: NUMPLANTA/SALA/SENSOR/***FREQ***
    
    - Value: int(segundos)

### General

- Activar/Desactivar todos los nodos de la sala:

    - Topic: NUMPLANTA/SALA/***ONOFF**
    
    - Value: 0 Desactivar, 1 Activar

- Activar/Desactivar todos los nodos de la planta:

    - Topic: NUMPLANTA/***ONOFF**
    
    - Value: 0 Desactivar, 1 Activar
    

## EXAMPLES

**2/3/TMP**

    Planta 2, sala 3, sensor temperatura

**2/3/TMP/ONOFF**

    Activar/Desactivar sensor de temperatura de la planta 2, sala 3.
    Depende del valor asociado a la publicación del topic.
    
**2/3/QOA/FREQ**

    Cambiar frecuencia de muestreo del sensor de calidad del aire de la planta 2 sala 3.
    




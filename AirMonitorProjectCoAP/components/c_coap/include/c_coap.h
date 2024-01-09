#ifndef __COAP_COM
#define __COAP_COM

#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(C_COAP_EVENT_BASE);

enum{
    C_COAP_EVENT_RECEIVED_DATA
};

typedef enum {
    C_COAP_SEND_TEMP,
    C_COAP_SEND_HUM,
    C_COAP_SEND_CO2,
    C_COAP_SEND_TVOC,
} CoapDataType;

void coap_start_client();

void coap_stop_client();

void save_device_token(char *device_token);
//Publish data
int coap_send_data(char *data, CoapDataType type);

#endif
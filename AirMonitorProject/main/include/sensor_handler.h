
#include "esp_event.h"
#include "c_sensorSI7021.h"
#include "c_sensorSGP30.h"

enum {
    SI7021_TMP = 0,
    SI7021_HUM = 1,
    SGP30_ECO2 = 2,
    SGP30_TVOC = 3,
};

void sensorSI7021_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);
void sensorSGP30_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);
void handler_get_sensor_stat(char *request_id);
void handler_set_sensor_stat(char * payload, char* response_topic);


void handler_set_publish_frequency(int freq, int sensor);
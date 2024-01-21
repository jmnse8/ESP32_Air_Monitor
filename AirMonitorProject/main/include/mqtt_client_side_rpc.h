#ifndef __MQTT_CLIENT_SIDE_RPC
#define __MQTT_CLIENT_SIDE_RPC
void request_publish_frequency();
void publish_frequency_response_handler(char * payload);

#endif
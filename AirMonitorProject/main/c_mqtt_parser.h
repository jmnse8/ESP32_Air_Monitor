#ifndef __MQTT_PARSER
#define __MQTT_PARSER

enum{
    C_MQTT_INVALID_TOPIC,
    C_MQTT_FREQ_TOPIC,
    C_MQTT_ONOFF_TOPIC
};

int mqtt_topic_parser(char *topic);
int parse_int_data(char *data);

#endif
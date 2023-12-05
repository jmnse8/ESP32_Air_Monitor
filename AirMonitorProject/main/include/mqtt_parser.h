#ifndef __MQTT_PARSER
#define __MQTT_PARSER

enum{
    MQTT_INVALID_TOPIC,
    MQTT_FREQ_TOPIC,
    MQTT_ONOFF_TOPIC,
    MQTT_MODE_TOPIC
};

int mqtt_topic_parser(char *topic);
int parse_int_data(char *data);

#endif
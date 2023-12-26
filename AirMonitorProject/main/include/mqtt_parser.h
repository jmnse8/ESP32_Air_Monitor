#ifndef __MQTT_PARSER
#define __MQTT_PARSER

enum{
    MQTT_INVALID_VALUE = -1,
    MQTT_FREQ_TOPIC,
    MQTT_ONOFF_TOPIC,
    MQTT_MODE_TOPIC
};

int mqtt_topic_parser(char *topic);
int parse_int_data(char *data);
char *build_topic(char *base, char* comp);
int its_for_me(const char *payload);
int parse_method(const char *payload);
int parse_bool_value(const char *payload);

#endif
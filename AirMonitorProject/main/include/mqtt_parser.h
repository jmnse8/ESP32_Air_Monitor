#ifndef __MQTT_PARSER
#define __MQTT_PARSER

enum{
    MQTT_INVALID_VALUE = -1,
    MQTT_GET_FREQ_TOPIC,
    MQTT_GET_ONOFF_TOPIC,
    MQTT_GET_MODE_TOPIC,

    MQTT_SET_FREQ_TOPIC,
    MQTT_SET_ONOFF_TOPIC,
    MQTT_SET_MODE_TOPIC,
};

char *mqtt_topic_last_token(char *topic);
int parse_int_data(char *data);
char *build_topic(char *base, char* comp);
int its_for_me(const char *payload);
int parse_method(const char *payload);
int parse_bool_value(const char *payload);
int parse_int_value(const char *payload);

#endif
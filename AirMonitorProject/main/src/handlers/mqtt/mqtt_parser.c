#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include "esp_log.h"
#include "mqtt_parser.h"


static int parse_topic(char *topic){

    // Compare the string
    if (strcmp(topic, "FREQ") == 0) {
        return C_MQTT_FREQ_TOPIC;
    } 
    else if (strcmp(topic, "ONOFF") == 0){
        return C_MQTT_ONOFF_TOPIC;
    }
    else{
        return C_MQTT_INVALID_TOPIC;
    }
}



int mqtt_topic_parser(char *topic){

    char *last_token = strrchr(topic, '/') + 1;

    return parse_topic(last_token);
}


int parse_int_data(char *data){
    // Attempt to convert str1 to an integer
    char *endptr1;
    int res = (int)strtol(data, &endptr1, 10);

    // Check if conversion was successful
    if (*endptr1 == '\0') {
        return res;
    }
    return -1;
}


#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include "esp_log.h"
#include "c_mqtt_parser.h"


static int parse_topic(char *topic){

    // Compare the string
    if (strcmp(topic, "FREQ") == 0) {
        printf("%s is FREQ\n", topic);
        return C_MQTT_FREQ_TOPIC;
    } 
    else if (strcmp(topic, "ONOFF") == 0){
        printf("%s is FREQ\n", topic);
        return C_MQTT_ONOFF_TOPIC;
    }
    else{
        printf("%s is an invalid topic\n", topic);
        return C_MQTT_INVALID_TOPIC;
    }
}



int mqtt_topic_parser(char *topic){

    char *last_token = strrchr(topic, '/');

    return parse_topic(last_token);


    /*
    const char *pattern = "([^/]+)/([^/]+)/([^/]+)/([^/]+)";

    regex_t regex;
    regmatch_t match[5];

    // Compile the regular expression
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        fprintf(stderr, "Error compiling regex\n");
        return 1;
    }

    if (regexec(&regex, topic, 5, match, 0) == 0) {
        // match[4] contains the fourth captured group
        int start = match[4].rm_so;
        int end = match[4].rm_eo;
        printf("Fourth argument in %s: %.*s\n", topic, end - start, topic + start);

        // Store the fourth argument in a string
        char result[50];
        strncpy(result, topic + start, end - start);
        result[end - start] = '\0';

        return parse_topic(result);


    } else {
        fprintf(stderr, "No match found in %s\n", topic);
    }

    // Free the compiled regular expression
    regfree(&regex);
    */

    return 0;   
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


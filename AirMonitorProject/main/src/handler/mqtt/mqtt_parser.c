#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include "esp_log.h"
#include "mqtt_parser.h"


static int parse_topic(char *topic){

    if (strcmp(topic, "FREQ") == 0) {
        return MQTT_FREQ_TOPIC;
    } 
    else if (strcmp(topic, "ONOFF") == 0){
        return MQTT_ONOFF_TOPIC;
    }
    else if (strcmp(topic, "MODE") == 0){
        return MQTT_MODE_TOPIC;
    }
    else{
        return MQTT_INVALID_TOPIC;
    }
}

char *build_topic(char *base, char* comp){
    if (base != NULL) {
        // Calculate the length of the resulting string
        int length = strlen(base) + strlen(comp);

        // Allocate memory for the concatenated string +1 for null terminator
        char* context_topic = (char*)malloc(length + 1); 

        // Copy the original string to the new buffer
        strcpy(context_topic, base);

        // Concatenate "/+" to the string
        strcat(context_topic, comp);

        printf("%s\n", context_topic);

        return context_topic;
    }

    return NULL;
}


/*
int mqtt_topic_parser(char *topic){

    int max_size = 10;
    char delim[] = "/";
    char *tokens[max_size];
    int index = 0;

    char *ptr = strtok(topic, delim);

    while(ptr != NULL && index < max_size){
		//printf("'%s'\n", ptr);
        tokens[index] = strdup(ptr);
        index++;
		ptr = strtok(NULL, delim);
	}

    for(int i=0; i<index; i++){
        printf("%s\n", tokens[i]);
    }

    //return MQTT_INVALID_TOPIC;

}
*/


int mqtt_topic_parser(char *topic){

    char *last_token = strrchr(topic, '/') + 1;

    return parse_topic(last_token);
}


int parse_int_data(char *data){
    // Attempt to convert str1 to an integer
    char *endptr;
    int res = (int)strtol(data, &endptr, 10);

    // Check if conversion was successful
    if (*endptr == '\0') {
        return res;
    }
    return -1;
}


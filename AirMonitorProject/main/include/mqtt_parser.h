#ifndef __MQTT_PARSER
#define __MQTT_PARSER

enum MQTT_TB_CMD {
    MQTT_INVALID_VALUE = -1,
    MQTT_OK = 0,
    MQTT_GET_FREQ_TOPIC,
    MQTT_GET_ONOFF_TOPIC,
    MQTT_GET_MODE_TOPIC,

    MQTT_GET_SENSOR_STAT_TOPIC,
    MQTT_SET_SENSOR_STAT_TOPIC,

    MQTT_SET_PUB_FREQ_TOPIC,
    MQTT_SET_ONOFF_TOPIC,
    MQTT_SET_MODE_TOPIC,

    MQTT_SET_CTX,
    MQTT_SET_PROV_TOKEN,

    MQTT_OTA_UPDATE_SETUP,
    MQTT_OTA_UPDATE_DATA,



    TB_TOPIC_ATTR_REQ,      //  v1/devices/me/attributes
    TB_TOPIC_ATTR_RESP,     //  v1/devices/me/attributes/response

    TB_TOPIC_PROV_RESP,     //  /provision/response

    TB_TOPIC_RPC_REQ,        // v1/devices/me/rpc/request/{request_id}
    TB_TOPIC_RPC_RESP,        // v1/devices/me/rpc/response/request_id}
};


int parse_topic(const char *topic);

/**
 * @brief Get the topic's last token.
*/
char *mqtt_topic_last_token(char *topic);

/**
 * @brief string 2 int
*/
int parse_int_data(char *data);

/**
 * @brief put base + comp together in a string. Don't forget to free the string
 * @return base + comp
*/
char *build_topic(char *base, char* comp);

/**
 * @brief Check if the message's context data matches the node's.
 * Currently unused
*/
int its_for_me(const char *payload);


int parse_method(const char *payload);


int parse_params_bool_value(const char *payload);


int parse_params_int_value(const char *payload);


int parse_params_str_value(const char* payload, char** str);

/**
 * @brief Builds the provisioning request JSON to send to TB
 * @return JSON containing the device provisioning request. Don't forget to free
*/
char *build_TB_prov_request();

/**
 * @brief Parse TB's device provisioning request to get the access token
 * @return Access token. Don't forget to free
*/
char *get_access_token_TB_response(char *payload);

#endif
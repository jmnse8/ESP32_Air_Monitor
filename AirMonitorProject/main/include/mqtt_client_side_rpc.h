#ifndef __MQTT_CLIENT_SIDE_RPC
#define __MQTT_CLIENT_SIDE_RPC

/**
 *  @brief   Send a client-side RPC request to get all sensor parameter's freq
*/
void request_publish_frequency();

/**
 *  @brief  Handler for the RPC request sent through request_publish_frequency
*/
void publish_frequency_response_handler(char * payload);

/**
 *  @brief  Send a client-side RPC request to get this node's floor/room context
*/
void request_node_context();

/**
 *  @brief  Handler for the RPC request sent through request_node_context
*/
void ctx_response_handler(char * payload);

#endif
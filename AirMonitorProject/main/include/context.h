#ifndef __CONTEXT_H
#define __CONTEXT_H


enum {
    CONTEXT_OFF = 0,
    CONTEXT_ON = 1
};

enum NODE_STATE_ENUM{
    NODE_STATE_BLANK = 0,                   //Factory reset (no provisioning) 

    /**
     * NODE_STATE_PROV: Waiting for provisioning:
     *      - Wifi's SSID and pwd 
     *      - Access token to thingsboard
    */
    NODE_STATE_PROV = 1,               

    /**
     * NODE_STATE_SIGUP_DEVICE2TB: Signing in TB in case access token was not 
     *                                  provided from NODE_STATE_PROV_WIFI state
    */
    NODE_STATE_SIGUP_DEVICE2TB,

    /**
     * NODE_STATE_HAS_WIFI_CREDENTIALS: Has Wifi ssid & pwd. Now checking access access token
    */
    NODE_STATE_HAS_WIFI_CREDENTIALS,

    /**
     * NODE_STATE_HAS_TB_TOKEN: Has WiFi credentials and TB Access token
    */
    NODE_STATE_HAS_TB_TOKEN,

    /**
     * NODE_STATE_WAIT_CTX: Waiting to be provided of the node's context data (floor/room)
    */
    NODE_STATE_WAIT_CTX,

    /**
     * NODE_STATE_REGULAR: Has WiFi connection, access token and context
    */
    NODE_STATE_REGULAR,

    /**
     * NODE_STATE_INVALID_DATA: If any of the provisioned data is invalid
    */
    NODE_STATE_INVALID_DATA,


    NODE_CONTROL_PARSE,

};

extern int NODE_STATUS;
extern char *NODE_TB_TOKEN;
extern char *NODE_SW_VERSION;


int context_is_invalid_ctx();

/**
 * @brief Check & Refresh node's current status.
 * @param status (NODE_STATE_ENUM or NULL)
 *          - If status == NULL, refreshes node's status automatically.
 * @return NODE_STATE_ENUM
 */
int context_refresh_node_status(int status);

/**
 * @brief Check if provided sw version ("a.b.c") is greater than current version.
*/
int context_check_sw_version(const char* ver);

/**
 * @brief Set ThingsBoard provisioning token (and store it in NVS).
 */
void context_set_tb_access_token(char * token);

/**
 * @brief Get ThingsBoard provisioning token (if stored in NVS).
 * @return TOKEN or NULL
 */
char *context_get_tb_access_token();

/**
 * @brief Set node floor/room context
 * @param
 *      save: If 1, save it in nvs
*/
void context_set_node_ctx(char *c, int save);

/**
 * @brief Get node floor/room context
*/
char *context_get_node_ctx();

/**
 * @brief Get node's state
*/
int context_get_node_status();

/**
 * @brief Check if node context == ctx
*/
int context_it_is_i(char * ctx);

/**
 * @brief Check if node is on or off
*/
int context_get_onoff();

/**
 * @brief Set onoff status
*/
void context_set_onoff(int onoff);

#endif
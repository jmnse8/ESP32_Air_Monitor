#ifndef __CONTEXT_H
#define __CONTEXT_H

enum {
    CONTEXT_OFF = 0,
    CONTEXT_ON = 1
};

enum NODE_STATE_ENUM{
    NODE_STATE_BLANK,                   //Factory reset (no provisioning) 
    NODE_STATE_PROV_WIFI,               //Provisioning wifi

    NODE_STATE_SIGUP_DEVICE2TB_STATE,   //Signing tp to TB

    NODE_STATE_HAS_TB_TOKEN,            //Wifi + Has TB token
    NODE_STATE_REGULAR,                 // Wifi + TB connection

};

extern int NODE_STATUS;
extern char *NODE_TB_TOKEN;

/**
 * @brief Check & Refresh node's current status.ç
 * @param status (NODE_STATE_ENUM or NULL)
 *          - If status == NULL, refreshes node's status automatically.
 */
void context_refresh_node_status(int status);

/**
 * @brief Set ThingsBoard provisioning token (and store it in NVS).
 */
void context_set_node_tb_token(char * token);

/**
 * @brief Get ThingsBoard provisioning token (if stored in NVS).
 * @return TOKEN or NULL
 */
char *context_get_node_tb_token();

/**
 * @brief Set node floor/room context
*/
void context_set_node_ctx(char *c);

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
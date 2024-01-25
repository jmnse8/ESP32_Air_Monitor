#ifndef __NVS_COM
#define __NVS_COM

enum NVS_RESPONSE_ENUM {
    NVS_OK,
    NVS_ERR,
    NVS_KEY_NOT_FOUND,
};

/**
 * @brief Initialize this component. Open Storage
*/
void nvs_init();

/**
 * @brief Write the key-value pair in NVS
 * @return NVS_RESPONSE_ENUM
*/
int nvs_write_string(char *key, char *value);

/**
 * @brief Read the value associated with key and store it in str
 * @return NVS_RESPONSE_ENUM
*/
int nvs_read_string(char *key, char **str);

#endif
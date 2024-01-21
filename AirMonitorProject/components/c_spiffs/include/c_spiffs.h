#ifndef __C_SPIFFS
#define __C_SPIFFS

#include <stdio.h>

enum SPIFFS_WRITE_MODE {
    SPIFFS_WRITE,   //Truncate all data in file (if exists)
    SPIFFS_APPEND,  //Append to file (if exists)
};

enum SPIFFS_LOG_TYPE {
    SPIFFS_LOGE = 0,
    SPIFFS_LOGW = 1,
    SPIFFS_LOGI = 2,
    SPIFFS_LOGD = 3,
    SPIFFS_LOGV = 4,

    SPIFFS_CONTROL
};

/**
 *  @brief  Init this component. Call this function first
*/
void spiffs_init(void);

/**
 *  @brief  Set ESP_LOG types to be stored in SPI. 
 *  @param  level
 *          Use enum SPIFFS_LOG_TYPE
*/
void spiffs_activate_level2log(int level);

/**
 *  @brief  Use this function as argument in esp_log_set_vprintf()
*/
int spiffs_log_vprintf(const char *fmt, va_list args);

/**
 *  @brief  Read the content of filename, stored in the filesystem
*/
void spiffs_read( char *filename);

/**
 *  @brief  Write data directly into the filesystem
 *  @param  data
 *          The data to write
 *  @param  filename
 *          The name of the file to write into
 *  @param  mode
 *          To trunc or to append. See enum SPIFFS_WRITE_MODE
*/
void spiffs_write(char *data, char *filename, int mode);

#endif
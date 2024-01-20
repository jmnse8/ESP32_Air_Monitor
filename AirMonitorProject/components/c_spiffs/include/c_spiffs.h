#ifndef __C_SPIFFS
#define __C_SPIFFS

#include <stdio.h>

enum {
    SPIFFS_WRITE,
    SPIFFS_APPEND,
};

void spiffs_init(void);
//void spiffs_set_log_file(char *filename);
int spiffs_log_vprintf(const char *fmt, va_list args);
void spiffs_read( char *filename);
void spiffs_write(char *data, char *filename, int mode);

#endif
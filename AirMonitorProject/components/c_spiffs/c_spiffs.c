#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include "c_spiffs.h"

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

#define APPEND_MODE "a+"
#define WRITE_MODE "w"
#define READ_MODE "r"

static const char *TAG = "C_SPIFFS";
static const char *SPIFFS_DIR = CONFIG_SPIFFS_DIRECTORY;
static const char *SPIFFS_LOGFILE = CONFIG_SPIFFS_LOG_FILE;

static bool static_fatal_error = false;
static const uint32_t WRITE_CACHE_CYCLE = 5;
static uint32_t counter_write = 0;

/**
 *  {
 *      [0] = ESP_LOGE,
 *      [1] = ESP_LOGW,
 *      [2] = ESP_LOGI, 
 *      [3] = ESP_LOGD,
 *      [4] = ESP_LOGV
 *  }
*/
static uint8_t SPIFFS_LOG_REG[] = {0, 0, 0, 0, 0};
static char SPIFFS_LOG_REF[] = {'E', 'W', 'I', 'D', 'V'};


void spiffs_activate_level2log(int level){
    if(level > 0){
        SPIFFS_LOG_REG[level % SPIFFS_CONTROL] = 1;
    }
}

int _needs2b_stored(char type){
    for(int i=0; i<5; i++){
        if(type == SPIFFS_LOG_REF[i]){
            return SPIFFS_LOG_REG[i];
        }
    }
    return 0;
}

/**
 * @brief This function will be called by the ESP log library every time ESP_LOG needs to be performed.
 * @important Do NOT use the ESP_LOG* macro's in this function ELSE recursive loop and stack overflow! So use printf() instead for debug messages.
 * @source https://www.esp32.com/viewtopic.php?t=3960
*/
int spiffs_log_vprintf(const char *fmt, va_list args) {

    char filedir[50];
    sprintf(filedir, "%s/%s", SPIFFS_DIR, SPIFFS_LOGFILE);

    FILE* f = fopen(filedir, APPEND_MODE);
    // #1 Write to SPIFFS
    if (f == NULL) {
        printf("%s() ABORT. file handle SPIFFS_LOGFILE is NULL\n", __FUNCTION__);
        return -1;
    }
    if (!static_fatal_error) {
        char esp_log_type = fmt[7];
        if(_needs2b_stored(esp_log_type)){
            int iresult = vfprintf(f, fmt, args);
            if (iresult < 0) {
                printf("%s() ABORT. failed vfprintf() -> disable future vfprintf(f) \n", __FUNCTION__);
                static_fatal_error = true;
                fclose(f);
                return iresult;
            }

            // #2 Smart commit after x writes
            counter_write++;
            if (counter_write % WRITE_CACHE_CYCLE == 0) {
                fsync(fileno(f));
            }

        }
    }
    
    fclose(f);
    // #3 ALWAYS Write to stdout!
    return vprintf(fmt, args);
}


void spiffs_write(char *data, char *filename, int mode){

    ESP_LOGI(TAG, "Opening file %s", filename);

    char filedir[50];
    sprintf(filedir, "%s/%s", SPIFFS_DIR, filename);

    char *m = (mode == SPIFFS_WRITE) ? WRITE_MODE : APPEND_MODE;

    FILE* f = fopen(filedir, m);
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open %s for writing", filedir);
        return;
    }
    fprintf(f, data);
    fprintf(f, "\n");
    fclose(f);

    ESP_LOGI(TAG, "File written");
}

void spiffs_read( char *filename){

    ESP_LOGI(TAG, "Reading file %s", filename);
    printf("------------------\n");
    char filedir[50];
    sprintf(filedir, "%s/%s", SPIFFS_DIR, filename);

    FILE* f = fopen(filedir, READ_MODE);
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char buffer[64];
    // Read lines from the file until there's no more text
    while (fgets(buffer, sizeof(buffer), f) != NULL) {
        printf("%s", buffer);
    }

    if (feof(f)) {
        printf("End of file reached.\n");
    } else if (ferror(f)) {
        perror("Error reading from file");
    }

    fclose(f);
}

void spiffs_init(void) {
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = SPIFFS_DIR,
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

#ifdef CONFIG_EXAMPLE_SPIFFS_CHECK_ON_START
    ESP_LOGI(TAG, "Performing SPIFFS_check().");
    ret = esp_spiffs_check(conf.partition_label);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
        return;
    } else {
        ESP_LOGI(TAG, "SPIFFS_check() successful");
    }
#endif

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
        esp_spiffs_format(conf.partition_label);
        return;
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    // Check consistency of reported partiton size info.
    if (used > total) {
        ESP_LOGW(TAG, "Number of used bytes cannot be larger than total. Performing SPIFFS_check().");
        ret = esp_spiffs_check(conf.partition_label);
        // Could be also used to mend broken files, to clean unreferenced pages, etc.
        // More info at https://github.com/pellepl/spiffs/wiki/FAQ#powerlosses-contd-when-should-i-run-spiffs_check
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
            return;
        } else {
            ESP_LOGI(TAG, "SPIFFS_check() successful");
        }
    }

}

void spiffs_deinit(){
    esp_vfs_spiffs_unregister(/*conf.partition_label*/NULL);
    ESP_LOGI(TAG, "SPIFFS unmounted");
}

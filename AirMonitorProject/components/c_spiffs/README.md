## SPIFFS Component
This component stores ESP_LOGx into the SPI filesystem.

### How to use
```
#include "esp_log.h"
```

```
// Initialize this component
spiffs_init();

// Set ESP_LOG's print function to the one this component has
esp_log_set_vprintf(&spiffs_log_vprintf);

//  Set which type of LOG needs to be stored in SPI. In this case, Warning and Error Type
spiffs_activate_level2log(SPIFFS_LOGW);
spiffs_activate_level2log(SPIFFS_LOGE);

// Test it executing some logs
ESP_LOGE("AA", "QQQQ");
ESP_LOGW("AA", "AAAA");
ESP_LOGI("BB", "HHHH");

// Read what has been written into the SPI log file
spiffs_read(CONFIG_SPIFFS_LOG_FILE);
```

#### Output
```
E (3813) AA: QQQQ
W (3853) AA: AAAA
```

For more info, look at the header file [c_spiffs.h](./include/c_spiffs.h)
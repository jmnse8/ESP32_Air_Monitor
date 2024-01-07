idf_component_register(SRCS "src/si7021.c" "src/i2c_config.c" "src/c_sensorSI7021.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_timer esp_event)

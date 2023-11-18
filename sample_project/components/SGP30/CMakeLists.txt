idf_component_register(SRCS "sensorSGP30.c" "SGP30.c"
                    INCLUDE_DIRS "include"
                    REQUIRES I2C esp_timer esp_event)

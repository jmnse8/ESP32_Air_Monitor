idf_component_register(SRCS "c_SGP30.c" "c_sensorSGP30.c"
                    INCLUDE_DIRS "include"
                    REQUIRES c_I2C esp_timer esp_event)

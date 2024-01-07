idf_component_register(SRCS "c_mqtt.c"
                    INCLUDE_DIRS "include"
                    REQUIRES mqtt esp_wifi nvs_flash protocol_examples_common) 

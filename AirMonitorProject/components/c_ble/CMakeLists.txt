idf_component_register(SRCS "c_ble.c"
                    INCLUDE_DIRS "include"
                    REQUIRES nvs_flash esp_event bt driver)

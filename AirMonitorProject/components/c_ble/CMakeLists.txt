idf_component_register(SRCS "c_ble.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_event esp_timer nvs_flash esp_netif main)

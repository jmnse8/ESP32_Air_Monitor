idf_component_register(
    SRCS 
        "c_nvs.c"
    INCLUDE_DIRS "include"
    REQUIRES
        nvs_flash
)

idf_component_register(
    SRCS 
        "c_spiffs.c"                
    INCLUDE_DIRS "include"
    REQUIRES 
        spiffs
) 

target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DLOG_LOCAL_LEVEL=ESP_LOG_VERBOSE")



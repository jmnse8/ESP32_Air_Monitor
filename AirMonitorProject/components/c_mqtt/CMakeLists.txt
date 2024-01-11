idf_component_register(
    SRCS 
        "c_mqtt.c"                
    INCLUDE_DIRS "include"
    REQUIRES 
        mqtt esp_wifi nvs_flash protocol_examples_common

    EMBED_TXTFILES ${project_dir}/components/c_mqtt/certs/ca_cert.pem
) 



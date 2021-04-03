cmake_minimum_required(VERSION 3.11.0)

idf_component_register(
        SRCS src/aws_iot_mqtt_config.c src/aws_iot_root_ca.c
        INCLUDE_DIRS include
        REQUIRES esp_common log nvs_flash mqtt
)

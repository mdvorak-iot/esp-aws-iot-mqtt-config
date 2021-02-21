idf_component_register(
        SRC_DIRS .
        INCLUDE_DIRS .
)

if (CONFIG_EXAMPLE_AWS_IOT_MQTT_CONFIG)
    target_add_binary_data(${COMPONENT_TARGET} "certs/aws-root-ca.pem" TEXT)
    target_add_binary_data(${COMPONENT_TARGET} "certs/certificate.pem.crt" TEXT)
    target_add_binary_data(${COMPONENT_TARGET} "certs/private.pem.key" TEXT)
endif ()

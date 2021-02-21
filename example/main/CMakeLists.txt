idf_component_register(
        SRC_DIRS .
        INCLUDE_DIRS .
)

if (CONFIG_EXAMPLE_AWS_IOT_MQTT_CONFIG)
    # NOTE it is possible to use DER certificates, but you need to change TEXT to BINARY here
    target_add_binary_data(${COMPONENT_TARGET} "certs/certificate.pem.crt" TEXT)
    target_add_binary_data(${COMPONENT_TARGET} "certs/private.pem.key" TEXT)
endif ()

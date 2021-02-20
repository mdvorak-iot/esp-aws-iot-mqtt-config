#ifndef AWS_IOT_MQTT_CONFIG_H_
#define AWS_IOT_MQTT_CONFIG_H_

#include <esp_err.h>
#include <mqtt_client.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t aws_iot_mqtt_config_store(const esp_mqtt_client_config_t *mqtt_cfg);

#define AWS_IOT_MQTT_CONFIG_NVS_NAME "aws_iot_mqtt_config"
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_HOST "host"
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_PORT "port"
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_URI "uri"
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_ID "client_id"
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_CERT "client_cert"
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_KEY "client_key"
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_KEY_PASSWORD "clientkey_password"

#ifdef __cplusplus
}
#endif

#endif

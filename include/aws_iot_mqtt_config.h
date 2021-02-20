#ifndef AWS_IOT_MQTT_CONFIG_H_
#define AWS_IOT_MQTT_CONFIG_H_

#include <esp_err.h>
#include <mqtt_client.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t aws_iot_mqtt_config_load(esp_mqtt_client_config_t *mqtt_cfg);

esp_err_t aws_iot_mqtt_config_store(const esp_mqtt_client_config_t *mqtt_cfg);

/**
 * @brief Frees some memory allocated by the aws_iot_mqtt_config_load() function.
 *
 * @details Call this after esp_mqtt_client_init() or esp_mqtt_set_config() functions.
 *
 * @details Frees memory from: host, uri, client_id and clientkey_password attributes.
 * Certificates cannot be freed, since mqtt does not internally make a copy -
 * they must remain allocated during whole lifetime of a mqtt client.
 *
 * @param mqtt_cfg MQTT configuration object.
 */
void aws_iot_mqtt_config_free_unused(esp_mqtt_client_config_t *mqtt_cfg);

#define AWS_IOT_MQTT_CONFIG_NVS_NAME "aws_iot_mqtt_config"
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_HOST "host"
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_PORT "port"
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_URI "uri"
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_ID "client_id"
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_CERT "client_cert"
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_KEY "client_key"
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENTKEY_PASSWORD "clientkey_password"

#ifdef __cplusplus
}
#endif

#endif

#ifndef AWS_IOT_MQTT_CONFIG_H_
#define AWS_IOT_MQTT_CONFIG_H_

#include <esp_err.h>
#include <mqtt_client.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Loads MQTT configuration from NVS storage and stores it into MQTT config object.
 *
 * @details All loaded values are newly allocated using malloc() - they must be freed when not needed.
 * You can call aws_iot_mqtt_config_free_unused() after esp_mqtt_client_init(), which
 * frees some values. After closing MQTT client, remaining values can be freed.
 *
 * @details Loads following attributes from NVS:
 * host, port, uri, transport, client_id, client_cert_pem, client_cert_len,
 * client_key_pem, client_key_len, clientkey_password, clientkey_password_len
 *
 * @details Note: Does not load CA certificate (cert_pem, cert_len).
 *
 * @param mqtt_cfg Empty MQTT configuration object.
 * @return ESP_OK on success, otherwise error.
 *
 * @see aws_iot_mqtt_config_store
 * @see aws_iot_mqtt_config_free_unused
 */
esp_err_t aws_iot_mqtt_config_load(esp_mqtt_client_config_t *mqtt_cfg);

/**
 *
 *
 * @param mqtt_cfg Populated MQTT configuration object.
 * @return ESP_OK on success, otherwise error.
 */
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

/** @brief Name of a NVS section, passed to nvs_open() */
#define AWS_IOT_MQTT_CONFIG_NVS_NAME "aws_iot_mqtt"
/** @brief Key under MQTT host is stored */
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_HOST "host"
/** @brief Key under MQTT port is stored */
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_PORT "port"
/** @brief Key under MQTT url is stored - mutually exclusive with host and port */
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_URI "uri"
/** @brief Key under MQTT client ID is stored */
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_ID "client_id"
/** @brief Key under MQTT client certificate is stored */
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_CERT "client_cert"
/** @brief Key under MQTT client private key is stored */
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_KEY "client_key"
/** @brief Key under MQTT client password for client_key is stored */
#define AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENTKEY_PASSWORD "clientkey_pass"

#ifdef __cplusplus
}
#endif

#endif

#include "aws_iot_mqtt_config.h"
#include <esp_log.h>
#include <mqtt_client.h>
#include <nvs.h>

static const char TAG[] = "aws_iot_mqtt_config";

#define HANDLE_ERROR(expr, action)  \
    {                               \
        esp_err_t err_ = (expr);    \
        if (err_ != ESP_OK) action; \
    }                               \
    (void)0

static esp_err_t nvs_helper_set_blob_or_string(nvs_handle_t handle, const char *key, const char *value, size_t length)
{
    if (value == NULL)
    {
        return nvs_erase_key(handle, key);
    }

    if (length == 0)
    {
        // Must be string
        length = strlen(value);
    }

    return nvs_set_blob(handle, key, value, length);
}

static esp_err_t nvs_helper_set_string(nvs_handle_t handle, const char *key, const char *value)
{
    if (value == NULL)
    {
        return nvs_erase_key(handle, key);
    }
    return nvs_set_str(handle, key, value);
}

esp_err_t aws_iot_mqtt_config_store(const esp_mqtt_client_config_t *mqtt_cfg)
{
    esp_err_t err = ESP_FAIL;
    nvs_handle_t handle = 0;

    // Open
    HANDLE_ERROR(err = nvs_open(AWS_IOT_MQTT_CONFIG_NVS_NAME, NVS_READWRITE, &handle), goto error);

    // Store
    HANDLE_ERROR(err = nvs_helper_set_string(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_HOST, mqtt_cfg->host), goto error);
    HANDLE_ERROR(err = nvs_set_u32(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_PORT, mqtt_cfg->port), goto error);
    HANDLE_ERROR(err = nvs_helper_set_string(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_URI, mqtt_cfg->uri), goto error);
    HANDLE_ERROR(err = nvs_helper_set_string(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_ID, mqtt_cfg->client_id), goto error);
    HANDLE_ERROR(err = nvs_helper_set_blob_or_string(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_CERT, mqtt_cfg->client_cert_pem, mqtt_cfg->client_cert_len), goto error);
    HANDLE_ERROR(err = nvs_helper_set_blob_or_string(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_KEY, mqtt_cfg->client_key_pem, mqtt_cfg->client_key_len), goto error);
    HANDLE_ERROR(err = nvs_helper_set_blob_or_string(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_KEY_PASSWORD, mqtt_cfg->clientkey_password, mqtt_cfg->clientkey_password_len), goto error);

    // Commit
    err = nvs_commit(handle);

error:
    // Close and exit
    nvs_close(handle);
    return err;
}

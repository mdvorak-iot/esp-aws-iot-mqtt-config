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

inline static esp_err_t nvs_helper_erase_key(nvs_handle_t handle, const char *key)
{
    esp_err_t err = nvs_erase_key(handle, key);
    return err != ESP_ERR_NVS_NOT_FOUND ? err : ESP_OK; // ignore not found error
}

static esp_err_t nvs_helper_set_blob_or_string(nvs_handle_t handle, const char *key, const char *value, size_t length)
{
    if (value == NULL)
    {
        return nvs_helper_erase_key(handle, key);
    }

    if (length == 0)
    {
        // Must be string, include null terminator
        length = strlen(value) + 1;
    }

    return nvs_set_blob(handle, key, value, length);
}

static esp_err_t nvs_helper_set_string(nvs_handle_t handle, const char *key, const char *value)
{
    if (value == NULL)
    {
        return nvs_helper_erase_key(handle, key);
    }
    return nvs_set_str(handle, key, value);
}

static esp_err_t nvs_helper_get_string(nvs_handle_t handle, const char *key, const char **str, size_t *str_len)
{
    // Already contains something, this is probably an user error
    if (*str != NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Get value length, including null terminating character
    size_t value_len = 0;
    nvs_get_str(handle, key, NULL, &value_len); // ignore error

    if (value_len)
    {
        char *value = malloc(value_len);
        esp_err_t err = nvs_get_str(handle, key, value, &value_len);
        if (err != ESP_OK)
        {
            free(value);
            return err;
        }

        *str = value;
        if (str_len) *str_len = value_len - 1; // minus null terminating character
    }

    return ESP_OK;
}

static esp_err_t nvs_helper_get_blob(nvs_handle_t handle, const char *key, const char **blob, size_t *blob_len)
{
    // Already contains something, this is probably an user error
    if (*blob != NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Get value length, including null terminating character
    size_t value_len = 0;
    nvs_get_blob(handle, key, NULL, &value_len); // ignore error

    if (value_len)
    {
        char *value = malloc(value_len);
        esp_err_t err = nvs_get_blob(handle, key, value, &value_len); // this should not fail, if we already got length
        if (err != ESP_OK)
        {
            free(value);
            return err;
        }

        *blob = value;
        if (blob_len) *blob_len = value_len;
    }
    else
    {
        *blob = NULL;
        if (blob_len) *blob_len = 0;
    }

    return ESP_OK;
}

esp_err_t aws_iot_mqtt_config_load(esp_mqtt_client_config_t *mqtt_cfg)
{
    if (mqtt_cfg == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Open
    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open(AWS_IOT_MQTT_CONFIG_NVS_NAME, NVS_READONLY, &handle);
    if (err != ESP_OK)
    {
        return err;
    }

    // Load
    HANDLE_ERROR(err = nvs_helper_get_string(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_HOST, &mqtt_cfg->host, NULL), goto error);
    mqtt_cfg->port = 0; // reset and ignore error
    nvs_get_u32(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_PORT, &mqtt_cfg->port);
    HANDLE_ERROR(err = nvs_helper_get_string(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_URI, &mqtt_cfg->uri, NULL), goto error);
    HANDLE_ERROR(err = nvs_helper_get_string(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_ID, &mqtt_cfg->client_id, NULL), goto error);
    HANDLE_ERROR(err = nvs_helper_get_blob(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_CERT, &mqtt_cfg->client_cert_pem, &mqtt_cfg->client_cert_len), goto error);
    HANDLE_ERROR(err = nvs_helper_get_blob(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_KEY, &mqtt_cfg->client_key_pem, &mqtt_cfg->client_key_len), goto error);
    size_t clientkey_password_len = 0; // value length in struct is an int, not size_t, for some reason
    HANDLE_ERROR(err = nvs_helper_get_blob(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENTKEY_PASSWORD, &mqtt_cfg->clientkey_password, &clientkey_password_len), goto error);
    mqtt_cfg->clientkey_password_len = clientkey_password_len;

error:
    // Close and exit
    nvs_close(handle);
    ESP_LOGI(TAG, "config loaded: %d (%s)", err, esp_err_to_name(err));
    return err;
}

esp_err_t aws_iot_mqtt_config_store(const esp_mqtt_client_config_t *mqtt_cfg)
{
    if (mqtt_cfg == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Open
    nvs_handle_t handle = 0;
    esp_err_t err = nvs_open(AWS_IOT_MQTT_CONFIG_NVS_NAME, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        return err;
    }

    // Store
    HANDLE_ERROR(err = nvs_helper_set_string(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_HOST, mqtt_cfg->host), goto error);
    HANDLE_ERROR(err = nvs_set_u32(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_PORT, mqtt_cfg->port), goto error);
    HANDLE_ERROR(err = nvs_helper_set_string(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_URI, mqtt_cfg->uri), goto error);
    HANDLE_ERROR(err = nvs_helper_set_string(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_ID, mqtt_cfg->client_id), goto error);
    HANDLE_ERROR(err = nvs_helper_set_blob_or_string(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_CERT, mqtt_cfg->client_cert_pem, mqtt_cfg->client_cert_len), goto error);
    HANDLE_ERROR(err = nvs_helper_set_blob_or_string(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENT_KEY, mqtt_cfg->client_key_pem, mqtt_cfg->client_key_len), goto error);
    HANDLE_ERROR(err = nvs_helper_set_blob_or_string(handle, AWS_IOT_MQTT_CONFIG_NVS_KEY_CLIENTKEY_PASSWORD, mqtt_cfg->clientkey_password, mqtt_cfg->clientkey_password_len), goto error);

    // Commit
    err = nvs_commit(handle);

error:
    // Close and exit
    nvs_close(handle);
    ESP_LOGI(TAG, "config stored: %d (%s)", err, esp_err_to_name(err));
    return err;
}

void aws_iot_mqtt_config_free_unused(esp_mqtt_client_config_t *mqtt_cfg)
{
    if (mqtt_cfg == NULL)
    {
        return;
    }

    free((char *)mqtt_cfg->host);
    mqtt_cfg->host = NULL;

    free((char *)mqtt_cfg->uri);
    mqtt_cfg->uri = NULL;

    free((char *)mqtt_cfg->client_id);
    mqtt_cfg->client_id = NULL;

    free((char *)mqtt_cfg->clientkey_password);
    mqtt_cfg->clientkey_password = NULL;
    mqtt_cfg->clientkey_password_len = 0;

    ESP_LOGD(TAG, "config mem freed");
}

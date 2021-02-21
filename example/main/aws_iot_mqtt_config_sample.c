#include "aws_iot_mqtt_config.h"
#include <esp_log.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

static const char TAG[] = "example";

static bool mqtt_started = false;
static esp_mqtt_client_handle_t mqtt_client = NULL;

#ifdef CONFIG_EXAMPLE_AWS_IOT_MQTT_CONFIG
extern const uint8_t aws_root_ca_pem_start[] asm("_binary_aws_root_ca_pem_start");
extern const uint8_t aws_root_ca_pem_end[] asm("_binary_aws_root_ca_pem_end");
extern const uint8_t certificate_pem_crt_start[] asm("_binary_certificate_pem_crt_start");
extern const uint8_t certificate_pem_crt_end[] asm("_binary_certificate_pem_crt_end");
extern const uint8_t private_pem_key_start[] asm("_binary_private_pem_key_start");
extern const uint8_t private_pem_key_end[] asm("_binary_private_pem_key_end");
#endif

void setup_example()
{
#if CONFIG_EXAMPLE_AWS_IOT_MQTT_CONFIG
    ESP_LOGI(TAG, "updating stored aws config");
    esp_mqtt_client_config_t mqtt_cfg = {};

#ifdef CONFIG_EXAMPLE_AWS_IOT_MQTT_HOST
    mqtt_cfg.host = CONFIG_EXAMPLE_AWS_IOT_MQTT_HOST;
#endif
#ifdef CONFIG_EXAMPLE_AWS_IOT_MQTT_PORT
    mqtt_cfg.port = CONFIG_EXAMPLE_AWS_IOT_MQTT_PORT;
#endif
#ifdef CONFIG_EXAMPLE_AWS_IOT_MQTT_CLIENT_ID
    mqtt_cfg.client_id = CONFIG_EXAMPLE_AWS_IOT_MQTT_CLIENT_ID;
#endif
#ifdef CONFIG_EXAMPLE_AWS_IOT_MQTT_TRANSPORT
    mqtt_cfg.transport = CONFIG_EXAMPLE_AWS_IOT_MQTT_TRANSPORT;
#endif

    mqtt_cfg.cert_pem = (const char *)aws_root_ca_pem_start;
    mqtt_cfg.cert_len = aws_root_ca_pem_end - aws_root_ca_pem_start;

    mqtt_cfg.client_cert_pem = (const char *)certificate_pem_crt_start;
    mqtt_cfg.client_cert_len = certificate_pem_crt_end - certificate_pem_crt_start;

    mqtt_cfg.client_key_pem = (const char *)private_pem_key_start;
    mqtt_cfg.client_key_len = private_pem_key_end - private_pem_key_start;

    ESP_ERROR_CHECK(aws_iot_mqtt_config_store(&mqtt_cfg));
#endif
}

static void wifi_event_handler(__unused void *handler_args, esp_event_base_t event_base, int32_t event_id, __unused void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        if (!mqtt_started)
        {
            // Initial connection
            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_mqtt_client_start(mqtt_client));
            mqtt_started = true;
        }
        else
        {
            // Ignore error here
            esp_mqtt_client_reconnect(mqtt_client);
        }
    }
}

static void mqtt_event_handler(__unused void *handler_args, __unused esp_event_base_t event_base, __unused int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch (event->event_id)
    {
    case MQTT_EVENT_BEFORE_CONNECT:
        ESP_LOGI(TAG, "mqtt connecting...");
        break;

    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "mqtt successfully connected");
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGW(TAG, "mqtt error type: 0x%x", event->error_handle->error_type);
        break;
    default:
        break;
    }
}

static void setup_wifi()
{
    // Initialize WiFi
    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL, NULL));

    // Configure Wi-Fi, if set
#ifdef CONFIG_EXAMPLE_WIFI_SSID
    wifi_config_t wifi_cfg = {};
    strncpy((char *)wifi_cfg.sta.ssid, CONFIG_EXAMPLE_WIFI_SSID, sizeof(wifi_cfg.sta.ssid) - 1);
#ifdef CONFIG_EXAMPLE_WIFI_PASSWORD
    strncpy((char *)wifi_cfg.sta.password, CONFIG_EXAMPLE_WIFI_PASSWORD, sizeof(wifi_cfg.sta.password) - 1);
#endif

    if (wifi_cfg.sta.ssid[0] != '\0')
    {
        ESP_LOGI(TAG, "using pre-compiled wifi credentials: ssid=" CONFIG_EXAMPLE_WIFI_SSID);
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_cfg));
    }
#endif
}

void app_main()
{
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("aws_iot_mqtt_config", ESP_LOG_DEBUG);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Events
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Store config, if configured
    setup_example();

    // Initialize WiFi
    setup_wifi();

    // Initialize MQTT
    esp_mqtt_client_config_t mqtt_cfg = {};
    ESP_ERROR_CHECK(aws_iot_mqtt_config_load(&mqtt_cfg));
    ESP_LOGI(TAG, "mqtt host=%s, port=%u, client_id=%s", mqtt_cfg.host, mqtt_cfg.port, mqtt_cfg.client_id);

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (!mqtt_client)
    {
        ESP_LOGE(TAG, "failed to init mqtt client");
        return;
    }
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_event_handler, NULL));

    // Release allocated memory (only client_cert and client_key will be "leaked", since mqtt client does not copy those)
    aws_iot_mqtt_config_free_unused(&mqtt_cfg);

    // Connect to WiFi - MQTT will be started after initial connection
    ESP_ERROR_CHECK(esp_wifi_connect());

    // Setup complete
    ESP_LOGI(TAG, "started");
}

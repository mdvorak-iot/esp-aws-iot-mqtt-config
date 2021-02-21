# aws_iot_mqtt_config component

![platformio build](https://github.com/mdvorak-iot/esp-aws-iot-mqtt-config/workflows/platformio%20build/badge.svg)

Component for [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest). Provides functions to store and load
[MQTT config](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/mqtt.html#_CPPv424esp_mqtt_client_config_t)
attributes
to/from [NVS storage](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html).

Following attributes are managed:

* `host`
* `port`
* `uri`
* `cert_pem`, `cert_len`
* `client_cert_pem`, `client_cert_len`
* `client_key_pem`, `client_key_len`
* `clientkey_password`, `clientkey_password_len`

## Usage

When used as IDF component, add it as a Git submodule

```
git submodule add https://github.com/mdvorak-iot/esp-aws-iot-mqtt-config.git components/aws_iot_mqtt_config
```

When used as [platformio](https://platformio.org) library, add it to `platformio.ini`:

```
[env]
lib_deps = https://github.com/mdvorak-iot/esp-aws-iot-mqtt-config.git
```

### Code

See [example](./example/main/aws_iot_mqtt_config_sample.c).

Note that when attributes are loaded into config, they are newly allocated and should be freed when not needed.

`esp_mqtt_client_init()` method creates copy of some attributes, but does not copy certificates and key. Those must not
be freed as long as mqtt client exists.

```
esp_mqtt_client_config_t mqtt_cfg = {};
mqtt_cfg.cert_pem = AWS_IOT_ROOT_CA;
mqtt_cfg.cert_len = AWS_IOT_ROOT_CA_LEN;
ESP_ERROR_CHECK(aws_iot_mqtt_config_load(&mqtt_cfg));

esp_mqtt_client_handle_t mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
// ...

aws_iot_mqtt_config_free_unused(&mqtt_cfg);
ESP_ERROR_CHECK(esp_mqtt_client_start(mqtt_client));
```

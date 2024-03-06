#ifndef BASE_CONF_H
#define BASE_CONF_H

/**
 * Contiene le costanti di configurazione
 */

#define CONF_ADMIN_PASSWORD               "admin"
#define CONF_ADMIN_USERNAME               "admin"

#define CONF_ANALOG_READ_MAX              (4095)

#define CONF_DHT_READ_INTERVAL_MIN        (5000)

#define CONF_INPUT_READ_INTERVAL          (1000)
#define CONF_INPUT_READ_INTERVAL_MIN      (100)

#define CONF_LOG_LEVEL                    "i"

#define CONF_MONITOR_BAUD_RATE            (115200)

#define CONF_MQ2_READ_SAMPLE_TIMES        (5)
#define CONF_MQ2_READ_SAMPLE_INTERVAL     (50)
#define CONF_MQ2_RL_VALUE                 (5)

#define CONF_HTML_TITLE                   "ESP32"

#define CONF_NTP_SERVER                   "pool.ntp.org"
#define CONF_NTP_INTERVAL                 (300000)

#define CONF_OPENHAB_BUS_ITEM             "ESP32BUS"

#define CONF_PUBLISH_INTERVAL             (60000)
#define CONF_PUBLISH_INTERVAL_MIN         (60000)

#define CONF_RULES_TRIGGERS_SIZE          (10)

#define CONF_SCHEMA_CHANNEL_COUNT         (16)
#define CONF_SCHEMA_INPUT_COUNT           (5)
#define CONF_SCHEMA_OUTPUT_COUNT          (10)
#define CONF_SCHEMA_PIN_COUNT             (64)
#define CONF_SCHEMA_PWM_FRQ               (5000)
#define CONF_SCHEMA_PWM_RES               (10)

// Valore per Europe/Rome in https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv 
#define CONF_TIME_ZONE                    "CET-1CEST,M3.5.0,M10.5.0/3"

#define CONF_WEB_HTTP_PORT                (80)
#define CONF_WEB_UPLOAD_LIMIT             (10240)
#define CONF_WEB_URI_CONFIG               "/config"
#define CONF_WEB_URI_CONFIG_RESET         "/config/reset"
#define CONF_WEB_URI_CONFIG_UPLOAD        "/config/upload"
#define CONF_WEB_URI_FIRMWARE_UPDATE      "/firmware/update"
#define CONF_WEB_URI_PUBLISH              "/publish"
#define CONF_WEB_URI_RESET                "/reset"

#define CONF_WIFI_AP_IP                   "192.168.32.1"
#define CONF_WIFI_AP_PSWD                 "esp32"
#define CONF_WIFI_AP_SSID                 "esp32"
#define CONF_WIFI_CHECK_INTERVAL          (60000)
#define CONF_WIFI_CHECK_INTERVAL_MIN      (60000)
#define CONF_WIFI_CHECK_THRESHOLD         (300000)
#define CONF_WIFI_CONN_TIMEOUT_MS         (10000)
#define CONF_WIFI_COUNT                   (3)
#define CONF_WIFI_MODE                    (1)
#define CONF_WIFI_MODE_LIMIT              (180000)
#define CONF_WIFI_NAME                    "esp32"

#endif

#ifndef INCLUDE_BASE_CONF_H
#define INCLUDE_BASE_CONF_H

/**
 * Contiene le costanti di configurazione
 */

const PROGMEM uint16_t CONF_ANALOG_READ_MAX             = 4095;

#define CONF_BMP280
#ifdef CONF_BMP280
#endif

#define CONF_DHT
#ifdef CONF_DHT
const PROGMEM uint32_t CONF_DHT_READ_INTERVAL_MIN       = 5000;
#endif

const PROGMEM char *   CONF_LOG_LEVEL                   = "i";

const PROGMEM uint32_t CONF_MONITOR_BAUD_RATE           = 115200;

const PROGMEM uint8_t  CONF_SCHEMA_PIN_COUNT            = 40;

// Valore per Europe/Rome in https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
const PROGMEM char *   CONF_TIME_ZONE                   = "CET-1CEST,M3.5.0,M10.5.0/3";

#define CONF_WIFI
#ifdef CONF_WIFI

#define CONF_WEB
#ifdef CONF_WEB
const PROGMEM char *   CONF_WEB_ADMIN_PASSWORD          = "admin";
const PROGMEM char *   CONF_WEB_ADMIN_USERNAME          = "admin";
const PROGMEM char *   CONF_WEB_HTML_TITLE              = "ESP32";
const PROGMEM uint16_t CONF_WEB_HTTP_PORT               = 80;
const PROGMEM uint16_t CONF_WEB_REDIRECT_REFRESH_MIN    = 5000;
const PROGMEM uint16_t CONF_WEB_UPLOAD_LIMIT            = 10240;
const PROGMEM char *   CONF_WEB_URI_CONFIG              = "/config";
const PROGMEM char *   CONF_WEB_URI_CONFIG_RESET        = "/config/reset";
const PROGMEM char *   CONF_WEB_URI_CONFIG_UPLOAD       = "/config/upload";
const PROGMEM char *   CONF_WEB_URI_FIRMWARE_UPDATE     = "/firmware/update";
const PROGMEM char *   CONF_WEB_URI_RESET               = "/reset";
#endif

const PROGMEM char *   CONF_WIFI_AP_IP                  = "192.168.32.1";
const PROGMEM char *   CONF_WIFI_AP_PSWD                = "esp32";
const PROGMEM char *   CONF_WIFI_AP_SSID                = "esp32";
const PROGMEM uint32_t CONF_WIFI_CHECK_INTERVAL_MIN     = 60000;
const PROGMEM uint32_t CONF_WIFI_CHECK_THRESHOLD        = 300000;
const PROGMEM uint32_t CONF_WIFI_CONN_TIMEOUT_MS        = 10000;
const PROGMEM uint8_t  CONF_WIFI_COUNT                  = 3;
const PROGMEM uint8_t  CONF_WIFI_MODE                   = 1;
const PROGMEM uint32_t CONF_WIFI_MODE_LIMIT             = 180000;
const PROGMEM char *   CONF_WIFI_NAME                   = "esp32";
const PROGMEM char *   CONF_WIFI_NTP_SERVER             = "pool.ntp.org";
const PROGMEM uint32_t CONF_WIFI_NTP_INTERVAL           = 300000;
const PROGMEM uint32_t CONF_WIFI_NTP_INTERVAL_MIN       = 60000;

#endif

#endif

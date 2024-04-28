#pragma once

/**
 * Contiene le costanti di configurazione
 */

#if defined(ESP32)
#define PLATFORM_ESP32
#define PLATFORM_TITLE "esp32"
#elif defined(ESP8266)
#define PLATFORM_ESP8266
#define PLATFORM_TITLE "esp8266"
#endif

const PROGMEM char*    CONF_ADMIN_PASSWORD              = "admin";
const PROGMEM char*    CONF_ADMIN_USERNAME              = "admin";

// #define CONF_ADMIN_ARDUINO_OTA
#define CONF_ADMIN_WEB_OTA

#define CONF_BMP280
#ifdef CONF_BMP280
#endif

#define CONF_DHT
#ifdef CONF_DHT
const PROGMEM uint32_t CONF_DHT_READ_INTERVAL_MIN       = 5000;
#endif

const PROGMEM uint8_t  CONF_LOG_LEVEL                   = 3; // ESP_LOG_INFO

const PROGMEM uint32_t CONF_MONITOR_BAUD_RATE           = 115200;

#define CONF_NEOPIXEL
#ifdef CONF_NEOPIXEL
#define CONF_NEOPIXEL_TYPE NEO_GRB+NEO_KHZ800
#endif

const PROGMEM char *   CONF_OPENHAB_BUS_ITEM            = "external_bus";

const PROGMEM uint32_t CONF_THERMO_AUTO_INTERVAL        = 60000;
const PROGMEM uint16_t CONF_THERMO_REFRESH_MIN          = 1000;

// Valore per Europe/Rome in https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
const PROGMEM char*    CONF_TIME_ZONE                   = "CET-1CEST,M3.5.0,M10.5.0/3";

#define CONF_WIFI
#ifdef CONF_WIFI

#define CONF_WEB
#ifdef CONF_WEB
const PROGMEM char*    CONF_WEB_HTML_TITLE              = PLATFORM_TITLE;
const PROGMEM uint16_t CONF_WEB_HTTP_PORT               = 80;

#define CONF_WEB_HTTPS
#ifdef CONF_WEB_HTTPS
const PROGMEM char*    CONF_WEB_HTTPS_CERT              = "MIICGDCCAYECAQIwDQYJKoZIhvcNAQELBQAwVDELMAkGA1UEBhMCREUxCzAJBgNVBAgMAkJFMQ8wDQYDVQQHDAZCZXJsaW4xEjAQBgNVBAoMCU15Q29tcGFueTETMBEGA1UEAwwKbXljYS5sb2NhbDAeFw0yNDAyMTIxMjAwNTlaFw0zNDAyMDkxMjAwNTlaMFUxCzAJBgNVBAYTAkRFMQswCQYDVQQIDAJCRTEPMA0GA1UEBwwGQmVybGluMRIwEAYDVQQKDAlNeUNvbXBhbnkxFDASBgNVBAMMC2VzcDMyLmxvY2FsMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDND5F9H6zW+hIBsy3sV7/eE26Cp6pXBA37M8GGeFXYoeLHzNffdXYdXFQONIjSOxn5X3gTBY2YppBvgzH9TCiN8Y2tXH2r6QpBnZrfxJBQAP1RmNReE6RTJ+l/4tJ8LLBe7Hs+jyCq9sFwF8b09thrUb2q1IwrlK+tOa8sW77HXwIDAQABMA0GCSqGSIb3DQEBCwUAA4GBAGh9FURupOK9PrQvLvH68/3MA3ZGkuO7V+xHY9G5lEMrg5fFY3zKJt31gYL9PXyEcpQwJd9+NXDneBrKWXs5Bxoy/sEIKT8BDPqHc0SRoAFnznvfh9c218YfaqcypeVtv37v35V6tQeOwCsr54HTBjZlFCPT4IxEwP0+Ya5bpHwB";
const PROGMEM char*    CONF_WEB_HTTPS_CERT_KEY          = "MIICXgIBAAKBgQDND5F9H6zW+hIBsy3sV7/eE26Cp6pXBA37M8GGeFXYoeLHzNffdXYdXFQONIjSOxn5X3gTBY2YppBvgzH9TCiN8Y2tXH2r6QpBnZrfxJBQAP1RmNReE6RTJ+l/4tJ8LLBe7Hs+jyCq9sFwF8b09thrUb2q1IwrlK+tOa8sW77HXwIDAQABAoGBAK2MoYuORU/kcVzy8sj2MzKWq470rBvdGKAj24QdUDp6OF71JTbASOjsfU9QcwPKdV8yx4MBzicFwXQ/Sw2faSnEQtnJIanYhXsV8PZh62h5mfbnQPEHTzoY/LN/JXxn5e4hMNm6ruUBr0OdcOvxvAvRiBEC4m6xzBYlc2KRKW4xAkEA9H9envByR8JpUqNWwv9ah1Ou2rKkJGzdTYivfEIzEUs20dplO7pmWgq5mq9wmFzYWr+xtvSjHtFMsDulwrTsTQJBANa1PRTzwdloUildxmDPuiAQ0PAI22UtSAxlib0sJE5sAX7MHB0Eej2r3E3msESi4/VgkM1gzhs6H4HryRzt6FsCQQClY3NyOFxVxmDQkcUi6vEmEal6LtVx/mJFDG0ItQ8uZ56RnhZUrOaHijG7PWoA5u1DleB1Tk75jcM+g1rBtvkRAkEAh4571xujGwE7RnS3R+4+w/n6AGKHKhTxtSnyUaZp8b1NIGH7qfJmv0MfX2uzhFIXfJeQNKrW1efXdq6IFiS6mwJAX+w1DIXu44zFSgeiexFJQ7yNInI6vOLGUHb1i1v+/m5Xscy7TASIMLACJFculTRvvmlbQot0kSpb+RGs03OMfw==";
const PROGMEM uint16_t CONF_WEB_HTTPS_PORT              = 443;
const PROGMEM char*    CONF_WEB_HTTPS_NAME              = PLATFORM_TITLE;
#endif

const PROGMEM uint16_t CONF_WEB_REDIRECT_REFRESH_MIN    = 5000;
const PROGMEM uint16_t CONF_WEB_UPLOAD_LIMIT            = 10240;
const PROGMEM char*    CONF_WEB_URI_CONFIG              = "/config";
const PROGMEM char*    CONF_WEB_URI_FIRMWARE_UPDATE     = "/update";
const PROGMEM char*    CONF_WEB_URI_RESET               = "/reset";

#define CONF_WEB_USERS
#ifdef CONF_WEB_USERS
const PROGMEM char*    CONF_WEB_URI_LOGIN               = "/login";
const PROGMEM char*    CONF_WEB_URI_LOGOUT              = "/logout";
const PROGMEM char *   CONF_WEB_URI_REST_DATA           = "/rest/data";
#endif

#endif

const PROGMEM char*    CONF_WIFI_AP_IP                  = "192.168.32.1";
const PROGMEM char*    CONF_WIFI_AP_PSWD                = PLATFORM_TITLE;
const PROGMEM char*    CONF_WIFI_AP_SSID                = PLATFORM_TITLE;
const PROGMEM uint32_t CONF_WIFI_CHECK_INTERVAL_MIN     = 60000;
const PROGMEM uint32_t CONF_WIFI_CHECK_THRESHOLD        = 300000;
const PROGMEM uint32_t CONF_WIFI_CONN_TIMEOUT_MS        = 10000;
const PROGMEM uint8_t  CONF_WIFI_COUNT                  = 3;
const PROGMEM uint8_t  CONF_WIFI_MODE                   = 1;
const PROGMEM uint32_t CONF_WIFI_MODE_LIMIT             = 180000;
const PROGMEM char*    CONF_WIFI_NAME                   = PLATFORM_TITLE;
const PROGMEM char*    CONF_WIFI_NTP_SERVER             = "pool.ntp.org";
const PROGMEM uint32_t CONF_WIFI_NTP_INTERVAL           = 300000;
const PROGMEM uint32_t CONF_WIFI_NTP_INTERVAL_MIN       = 60000;

#endif

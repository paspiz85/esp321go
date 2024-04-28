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

const PROGMEM uint32_t CONF_INPUT_READ_INTERVAL         = 1000;
const PROGMEM uint32_t CONF_INPUT_READ_INTERVAL_MIN     = 100;

const PROGMEM uint8_t  CONF_LOG_LEVEL                   = 3; // ESP_LOG_INFO

const PROGMEM uint32_t CONF_MONITOR_BAUD_RATE           = 115200;

const PROGMEM uint8_t  CONF_MQ2_READ_SAMPLE_TIMES       = 5;
const PROGMEM uint32_t CONF_MQ2_READ_SAMPLE_INTERVAL    = 50;
const PROGMEM float    CONF_MQ2_RL_VALUE                = 5;

#define CONF_NEOPIXEL
#ifdef CONF_NEOPIXEL
#define CONF_NEOPIXEL_TYPE NEO_GRB+NEO_KHZ800
#endif

const PROGMEM char*    CONF_OPENHAB_BUS_ITEM            = "external_bus";

const PROGMEM uint32_t CONF_PUBLISH_INTERVAL            = 60000;
const PROGMEM uint32_t CONF_PUBLISH_INTERVAL_MIN        = 60000;

const PROGMEM uint8_t  CONF_RULES_TRIGGERS_SIZE         = 10;

const PROGMEM uint8_t  CONF_SCHEMA_INPUT_COUNT          = 5;
const PROGMEM uint8_t  CONF_SCHEMA_OUTPUT_COUNT         = 10;

// Valore per Europe/Rome in https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
const PROGMEM char*    CONF_TIME_ZONE                   = "CET-1CEST,M3.5.0,M10.5.0/3";

#define CONF_WIFI
#ifdef CONF_WIFI

#define CONF_WEB
#ifdef CONF_WEB
const PROGMEM char*    CONF_WEB_HTML_TITLE              = PLATFORM_TITLE;
const PROGMEM uint16_t CONF_WEB_HTTP_PORT               = 80;

#ifdef PLATFORM_ESP8266
// #define CONF_WEB_HTTPS
#ifdef CONF_WEB_HTTPS
static const char CONF_WEB_HTTPS_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIC6jCCAlOgAwIBAgIUZIw0cBcWDPJZe8ZIDu6bDqdwwvwwDQYJKoZIhvcNAQEL
BQAwejELMAkGA1UEBhMCUk8xCjAIBgNVBAgMAUIxEjAQBgNVBAcMCUJ1Y2hhcmVz
dDEbMBkGA1UECgwST25lVHJhbnNpc3RvciBbUk9dMRYwFAYDVQQLDA1PbmVUcmFu
c2lzdG9yMRYwFAYDVQQDDA1lc3A4MjY2LmxvY2FsMB4XDTE5MDQxMzE1NTMzOFoX
DTIwMDQxMjE1NTMzOFowejELMAkGA1UEBhMCUk8xCjAIBgNVBAgMAUIxEjAQBgNV
BAcMCUJ1Y2hhcmVzdDEbMBkGA1UECgwST25lVHJhbnNpc3RvciBbUk9dMRYwFAYD
VQQLDA1PbmVUcmFuc2lzdG9yMRYwFAYDVQQDDA1lc3A4MjY2LmxvY2FsMIGfMA0G
CSqGSIb3DQEBAQUAA4GNADCBiQKBgQCiZmrefwe6AwQc5BO+T/18IVyJJ007EASn
HocT7ODkL2HNgIKuQCnPimiysLh29tL1rRoE4v7qtpV4069BrMo2XqFvZkfbZo/c
qMcLJi43jSvWVUaWvk8ELlXNR/PX4627MilhC4bLD57VB7Q2AF4jrAVhBLzClqg0
RyCS1yab+wIDAQABo20wazAdBgNVHQ4EFgQUYvIljCgcnOfeRn1CILrj38c7Ke4w
HwYDVR0jBBgwFoAUYvIljCgcnOfeRn1CILrj38c7Ke4wDwYDVR0TAQH/BAUwAwEB
/zAYBgNVHREEETAPgg1lc3A4MjY2LmxvY2FsMA0GCSqGSIb3DQEBCwUAA4GBAI+L
mejdOgSCmsmhT0SQv5bt4Cw3PFdBj3EMFltoDsMkrJ/ot0PumdPj8Mukf0ShuBlL
alf/hel7pkwMbXJrQyt3+EN/u4SjjZZJT21Zbxbmo1BB/vy1fkugfY4F3JavVAQ/
F49UaclGs77AVkDYwKlRh5VWhmnfuXPN6NXkfV+z
-----END CERTIFICATE-----
)EOF";
static const char CONF_WEB_HTTPS_CERT_KEY[] PROGMEM =  R"EOF(
-----BEGIN PRIVATE KEY-----
MIICdwIBADANBgkqhkiG9w0BAQEFAASCAmEwggJdAgEAAoGBAKJmat5/B7oDBBzk
E75P/XwhXIknTTsQBKcehxPs4OQvYc2Agq5AKc+KaLKwuHb20vWtGgTi/uq2lXjT
r0GsyjZeoW9mR9tmj9yoxwsmLjeNK9ZVRpa+TwQuVc1H89fjrbsyKWELhssPntUH
tDYAXiOsBWEEvMKWqDRHIJLXJpv7AgMBAAECgYA5Syqu3mAKdt/vlWOFw9CpB1gP
JydvC+KoVvPOysY4mqLFjm4MLaTSjIENcZ1SkxewBubkDHVktw+atgvhfqVD4xnC
ewMpuN6Rku5A6EELhUoDrgMEt6M9D/0/iPaMm3VDtLXJq5SuKTpnM+vyE4/uM2Gu
4COfL4GQ0A5KWTzGcQJBANfpU/kwdZf8/oaOvpNZPGRsryjIXXuWMzKKM+M1RqSA
UQV596MGXjo8k8YG/A99rTmVhbeTMC2/7gIyGTePe/kCQQDAjZg2Ujz7wY3gf1Fi
ZETL7DHsss74sZyWZI490yIX0TQqKpXqEIKlkV+UZTOoSZiAaUyPjokblPmTkKfu
uMyTAkBIBjfS+o1fxC+L53Y/ZRc2UOMlcaFtpq8xftTMSGtmWL+uWf93zJoGR0rs
VkwjRsNQYEaY9Gqv+ESHSvsKg7zRAkEAoOLuhpzqVZThHe5jqumKzjS5dkPlScjl
xIeaji/msa3cf0r73goTj5HLIev5YKi1or3Y+a4oA4LTkifxGTcRvwJBAJB+qUE6
y8y+4yxStsWu362tn2o4EjyPL2UGc40wtlQng2GzPZ20+xVYcLxsJXE5/Jqg8IeI
elVVC46RfjDK9G0=
-----END PRIVATE KEY-----
)EOF";
const PROGMEM uint16_t CONF_WEB_HTTPS_PORT              = 443;
const PROGMEM char*    CONF_WEB_HTTPS_NAME              = PLATFORM_TITLE;
#endif
#endif

const PROGMEM uint16_t CONF_WEB_REDIRECT_REFRESH_MIN    = 5000;
const PROGMEM uint16_t CONF_WEB_UPLOAD_LIMIT            = 10240;
const PROGMEM char*    CONF_WEB_URI_CONFIG              = "/config";
const PROGMEM char*    CONF_WEB_URI_FIRMWARE_UPDATE     = "/update";
const PROGMEM char*    CONF_WEB_URI_PUBLISH             = "/publish";
const PROGMEM char*    CONF_WEB_URI_RESET               = "/reset";

#define CONF_WEB_USERS
#ifdef CONF_WEB_USERS
const PROGMEM char*    CONF_WEB_URI_LOGIN               = "/login";
const PROGMEM char*    CONF_WEB_URI_LOGOUT              = "/logout";
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

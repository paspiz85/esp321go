#ifndef INCLUDE_CONFIG_H
#define INCLUDE_CONFIG_H

/**
 * Gestione della configurazione.
 * 
 * @see https://github.com/arduino-libraries/Arduino_JSON/blob/master/src/JSON.h
 * @see https://github.com/vshymanskyy/Preferences/blob/main/src/Preferences.h
 */

#include "base_conf.h"
#include "base_utils.h"
#include <Arduino.h>
#include <Arduino_JSON.h>
#include <Preferences.h>

// ATTENZIONE: le chiavi troppo lunghe non vengono gestite (forse max 15 chars?)

#define PREF_ADMIN_USERNAME               "admin_username"
#define PREF_ADMIN_PASSWORD               "admin_password"

#define PREF_BLINK_LED_PIN                "blink_led_pin"

#define PREF_CONFIG_PUBLISH               "publish_conf"

#ifdef CONF_BMP280
#define PREF_BMP280_ADDR                  "bmp280_addr"
#endif

#ifdef CONF_DHT
#define PREF_DHT_PIN                      "dht_pin"
#define PREF_DHT_READ_INTERVAL            "dht_interval"
#define PREF_DHT_TYPE                     "dht_type"
#endif

#define PREF_LOG_LEVEL                    "log_level"

#define PREF_PREFIX_WIFI                  "wifi"            // Prefisso per le reti WiFi
#define PREF_PREFIX_WIFI_PSWD             "_pswd"           // Suffisso per le password WiFi
#define PREF_PREFIX_WIFI_SSID             "_ssid"           // Suffisso per gli SSID WiFi

#define PREF_REBOOT_FREE                  "reboot_free"
#define PREF_REBOOT_MS                    "reboot_ms"

#define PREF_TIME_ZONE                    "time_zone"

#ifdef CONF_WEB
#define PREF_WEB_HTML_TITLE               "html_title"
#endif

#ifdef CONF_WEB_HTTPS
#define PREF_WEB_CERT                     "web_cert"
#define PREF_WEB_CERT_KEY                 "web_cert_key"
#endif

#ifdef CONF_WIFI
#define PREF_WIFI_AP_IP                   "wifi0_ip"
#define PREF_WIFI_AP_PIN                  "wifi0_pin"
#define PREF_WIFI_AP_PSWD                 "wifi0_pswd"
#define PREF_WIFI_AP_SSID                 "wifi0_ssid"
#define PREF_WIFI_CHECK_INTERVAL          "wifi_check_ms"
#define PREF_WIFI_CHECK_THRESHOLD         "wifi_check_th"
#define PREF_WIFI_MODE                    "wifi_mode"
#define PREF_WIFI_NAME                    "wifi_name"
#endif

typedef struct config {
  const char * key;
  ctype_t type;
  const char * desc;
  int count;
  const struct config * refs;
  int refs_len;
} Config;

const Config config_wifi_defs[] = {
  { .key = PREF_PREFIX_WIFI_SSID,     .type = STRING, .desc = EMPTY },
  { .key = PREF_PREFIX_WIFI_PSWD,     .type = STRING, .desc = EMPTY }
};

const Config config_defs[] = {
  { .key = PREF_BLINK_LED_PIN,        .type = UINT8,  .desc = EMPTY },
  { .key = PREF_REBOOT_FREE,          .type = UINT32, .desc = "Abilita il riavvio se la memoria scende sotto la soglia (0 = funzione disabilitata)" },
  { .key = PREF_REBOOT_MS,            .type = UINT32, .desc = "Abilita il riavvio dopo N millis (0 = funzione disabilitata)" },
  { .key = PREF_TIME_ZONE,            .type = STRING, .desc = EMPTY },
#ifdef CONF_WIFI
  { .key = PREF_WIFI_MODE,            .type = UINT8,  .desc = "0 -> off\n 1 -> STATION\n 2 -> ACCESS_POINT\n others -> STATION" },
  { .key = PREF_WIFI_AP_IP,           .type = STRING, .desc = ("default "+String(CONF_WIFI_AP_IP)).c_str() },
  { .key = PREF_WIFI_AP_SSID,         .type = STRING, .desc = EMPTY },
  { .key = PREF_WIFI_AP_PSWD,         .type = STRING, .desc = EMPTY },
  { .key = PREF_WIFI_AP_PIN,          .type = UINT8,  .desc = "Uscita digitale attivata in modalità AP" },
  { .key = PREF_PREFIX_WIFI,          .type = DARRAY, .desc = EMPTY, .count = CONF_WIFI_COUNT, .refs = &config_wifi_defs[0], .refs_len = len_array(config_wifi_defs) },
  { .key = PREF_WIFI_CHECK_INTERVAL,  .type = UINT32, .desc = ("min "+String(CONF_WIFI_CHECK_INTERVAL_MIN)).c_str() },
  { .key = PREF_WIFI_CHECK_THRESHOLD, .type = UINT32, .desc = ("default "+String(CONF_WIFI_CHECK_THRESHOLD)+" (0 = funzione disabilitata)").c_str() },
  { .key = PREF_WIFI_NAME,            .type = STRING, .desc = EMPTY },
#endif
#ifdef CONF_WEB
  { .key = PREF_WEB_HTML_TITLE,       .type = STRING, .desc = EMPTY },
#endif
#ifdef CONF_WEB_HTTPS
  { .key = PREF_WEB_CERT,             .type = STRING, .desc = "Formato PEM solo parte Base64" },
  { .key = PREF_WEB_CERT_KEY,         .type = STRING, .desc = "Formato PEM solo parte Base64" },
#endif
  { .key = PREF_CONFIG_PUBLISH,       .type = BOOL,   .desc = EMPTY },
#ifdef CONF_BMP280
  { .key = PREF_BMP280_ADDR,          .type = UINT8,  .desc = "119 per 0x77 oppure 118 per 0x76" },
#endif
#ifdef CONF_DHT
  { .key = PREF_DHT_PIN,              .type = UINT8,  .desc = EMPTY },
  { .key = PREF_DHT_TYPE,             .type = UINT8,  .desc = EMPTY },
  { .key = PREF_DHT_READ_INTERVAL,    .type = UINT32, .desc = ("default e min "+String(CONF_DHT_READ_INTERVAL_MIN)).c_str() },
#endif
  { .key = PREF_LOG_LEVEL,            .type = STRING, .desc = "Livello di log (d,i,w,e)" }
};

Preferences preferences;

String preferences_get(const char * key, ctype_t type, bool emptyOnNull = false) {
  if (emptyOnNull && !preferences.isKey(key)) {
    return EMPTY;
  }
  switch (type) {
    case UINT8:  return String(preferences.getUChar(key));
    case UINT16: return String(preferences.getUShort(key));
    case UINT32: return String(preferences.getULong(key));
    case UINT64: return uint64_to_string(preferences.getULong64(key));
    case INT8:   return String(preferences.getChar(key));
    case INT16:  return String(preferences.getShort(key));
    case INT32:  return String(preferences.getLong(key));
    case INT64:  return int64_to_string((long long) preferences.getLong64(key));
    case STRING: return preferences.getString(key);
    case BOOL:   return preferences.getBool(key) ? "true" : "false";
    case FLOAT:  return String(preferences.getFloat(key));
    case DOUBLE: return String(preferences.getDouble(key));
    case STRUCT: return preferences.getString(key);
    default:     return EMPTY;
  }
}

bool preferences_remove(const char * key,ctype_t type, String publish_key = EMPTY) {
  log_d("removing %s",key);
  preferences.remove(key);
  if (publish_key != EMPTY) {
    if (type == STRING) {
      item_publish(publish_key.c_str(),EMPTY);
    } else if (type == BOOL) {
      item_publish(publish_key.c_str(),false);
    } else {
      item_publish(publish_key.c_str(),0);
    }
  }
  return true;
}

bool preferences_put(const char * key, ctype_t type, String value, String publish_key = EMPTY) {
  if (type == UINT8) {
    uint8_t number = str_to_uint8(value);
    log_d("saving %s = %hhu",key,number);
    preferences.putUChar(key,number);
    if (publish_key != EMPTY) {
      item_publish(publish_key.c_str(),number);
    }
  } else if (type == UINT16) {
    uint16_t number = str_to_uint16(value);
    log_d("saving %s = %" PRIu16,key,number);
    preferences.putUShort(key,number);
    if (publish_key != EMPTY) {
      item_publish(publish_key.c_str(),number);
    }
  } else if (type == UINT32) {
    uint32_t number = str_to_uint32(value);
    log_d("saving %s = %" PRIu32,key,number);
    preferences.putULong(key,number);
    if (publish_key != EMPTY) {
      item_publish(publish_key.c_str(),number);
    }
  } else if (type == UINT64) {
    uint64_t number = str_to_uint64(value);
    log_d("saving %s = %" PRIu64,key,number);
    preferences.putULong64(key,number);
    if (publish_key != EMPTY) {
      item_publish(publish_key.c_str(),uint64_to_string(number));
    }
  } else if (type == INT8) {
    int8_t number = str_to_int8(value);
    log_d("saving %s = %" PRIi8,key,number);
    preferences.putChar(key,number);
    if (publish_key != EMPTY) {
      item_publish(publish_key.c_str(),number);
    }
  } else if (type == INT16) {
    int16_t number = str_to_int16(value);
    log_d("saving %s = %" PRIi16,key,number);
    preferences.putShort(key,number);
    if (publish_key != EMPTY) {
      item_publish(publish_key.c_str(),number);
    }
  } else if (type == INT32) {
    int32_t number = str_to_int32(value);
    log_d("saving %s = %" PRIi32,key,number);
    preferences.putLong(key,number);
    if (publish_key != EMPTY) {
      item_publish(publish_key.c_str(),number);
    }
  } else if (type == INT64) {
    int64_t number = str_to_int64(value);
    log_d("saving %s = %" PRIi64,key,number);
    preferences.putLong64(key,number);
    if (publish_key != EMPTY) {
      item_publish(publish_key.c_str(),int64_to_string(number));
    }
  } else if (type == STRING) {
    log_d("saving %s = %s",key,value.c_str());
    preferences.putString(key,value);
    if (publish_key != EMPTY) {
      item_publish(publish_key.c_str(),value);
    }
  } else if (type == BOOL) {
    bool number = str_to_bool(value);
    log_d("saving %s = %s",key,value.c_str());
    preferences.putBool(key,number);
    if (publish_key != EMPTY) {
      item_publish(publish_key.c_str(),number);
    }
  } else if (type == FLOAT) {
    float number = str_to_float(value);
    log_d("saving %s = %f",key,number);
    preferences.putFloat(key,number);
    if (publish_key != EMPTY) {
      item_publish(publish_key.c_str(),number);
    }
  } else if (type == DOUBLE) {
    double number = str_to_double(value);
    log_d("saving %s = %lf",key,number);
    preferences.putDouble(key,number);
    if (publish_key != EMPTY) {
      item_publish(publish_key.c_str(),number);
    }
  } else if (type == STRUCT) {
    log_d("saving %s = %s",key,value.c_str());
    preferences.putString(key,value);
    if (publish_key != EMPTY) {
      item_publish(publish_key.c_str(),value);
    }
  } else {
    return false;
  }
  return true;
}

#endif

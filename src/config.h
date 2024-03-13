#ifndef INCLUDE_CONFIG_H
#define INCLUDE_CONFIG_H

/**
 * Gestione della configurazione.
 * 
 * @see https://github.com/arduino-libraries/Arduino_JSON/blob/master/src/JSON.h
 * @see https://github.com/espressif/arduino-esp32/blob/master/libraries/Preferences/src/Preferences.h
 */

#include "base_conf.h"
#include "base_macros.h"
#include "base_utils.h"
#include <Arduino.h>
#include <Arduino_JSON.h>
#include <Preferences.h>

// ATTENZIONE: le chiavi troppo lunghe non vengono gestite (forse max 15 chars?)

#define PREF_BOILER_PIN                   "boiler_pin"

#define PREF_CONFIG_PUBLISH               "publish_conf"

#ifdef CONF_DHT
#define PREF_DHT_PIN                      "dht_pin"
#define PREF_DHT_READ_INTERVAL            "dht_interval"
String  PREF_DHT_READ_INTERVAL_DESC =     "min "+String(CONF_DHT_READ_INTERVAL_MIN)+", default "+String(CONF_DHT_READ_INTERVAL_MIN);
#define PREF_DHT_TYPE                     "dht_type"
#endif

#define PREF_LOG_LEVEL                    "log_level"
#define PREF_LOG_LEVEL_DESC               "Livello di log (d,i,w,e)"

#define PREF_PREFIX_WIFI                  "wifi"            // Prefisso per le reti WiFi
#define PREF_PREFIX_WIFI_PSWD             "_pswd"           // Suffisso per le password WiFi
#define PREF_PREFIX_WIFI_SSID             "_ssid"           // Suffisso per gli SSID WiFi

#define PREF_REBOOT_FREE                  "reboot_free"
#define PREF_REBOOT_FREE_DESC             "Abilita il riavvio se la memoria scende sotto la soglia (0 = funzione disabilitata)"
#define PREF_REBOOT_MS                    "reboot_ms"
#define PREF_REBOOT_MS_DESC               "Abilita il riavvio dopo N millis (0 = funzione disabilitata)"

#define PREF_THERMO_AUTO_INTERVAL         "thermo_auto_interval_ms"
String  PREF_THERMO_AUTO_INTERVAL_DESC =  "default "+String(CONF_THERMO_AUTO_INTERVAL)+" ms";
#define PREF_THERMO_MODE                  "thermo_mode"
#define PREF_THERMO_REFRESH               "thermo_refresh"
String  PREF_THERMO_REFRESH_DESC =        "default and min "+String(CONF_THERMO_REFRESH_MIN)+" ms";
#define PREF_THERMO_TARGET                "thermo_target"

#define PREF_TIME_ZONE                    "time_zone"
String  PREF_TIME_ZONE_DESC =             "default "+String(CONF_TIME_ZONE);

#ifdef CONF_WEB
#define PREF_WEB_ADMIN_USERNAME           "admin_username"
String  PREF_WEB_ADMIN_USERNAME_DESC =    "default "+String(CONF_WEB_ADMIN_USERNAME);
#define PREF_WEB_ADMIN_PASSWORD           "admin_password"
String  PREF_WEB_ADMIN_PASSWORD_DESC =    "default "+String(CONF_WEB_ADMIN_PASSWORD);
#define PREF_WEB_HTML_TITLE               "html_title"
String  PREF_WEB_HTML_TITLE_DESC =        "Titolo dell'interfaccia HTTP/HTML, default "+String(CONF_WEB_HTML_TITLE);
#define PREF_WEB_USERS                    "users"
#endif

#ifdef CONF_WEB_HTTPS
#define PREF_WEB_CERT                     "web_cert"
#define PREF_WEB_CERT_DESC                "Formato PEM solo parte Base64"
#define PREF_WEB_CERT_KEY                 "web_cert_key"
#define PREF_WEB_SECURE                   "web_secure"
#define PREF_WEB_SECURE_DESC              "Abilita solo la connessione sicura"
#endif

#ifdef CONF_WIFI
#define PREF_WIFI_AP_IP                   "wifi0_ip"
String  PREF_WIFI_AP_IP_DESC =            "default "+String(CONF_WIFI_AP_IP);
#define PREF_WIFI_AP_PIN                  "wifi0_pin"
#define PREF_WIFI_AP_PIN_DESC             "Uscita digitale attivata in modalità AP"
#define PREF_WIFI_AP_PSWD                 "wifi0_pswd"
String  PREF_WIFI_AP_PSWD_DESC =          "default "+String(CONF_WIFI_AP_PSWD);
#define PREF_WIFI_AP_SSID                 "wifi0_ssid"
String  PREF_WIFI_AP_SSID_DESC =          "default "+String(CONF_WIFI_AP_SSID);
#define PREF_WIFI_CHECK_INTERVAL          "wifi_check_ms"
String  PREF_WIFI_CHECK_INTERVAL_DESC =   "min "+String(CONF_WIFI_CHECK_INTERVAL_MIN);
#define PREF_WIFI_CHECK_THRESHOLD         "wifi_check_th"
String  PREF_WIFI_CHECK_THRESHOLD_DESC =  "default "+String(CONF_WIFI_CHECK_THRESHOLD)+" (0 = funzione disabilitata)";
#define PREF_WIFI_MODE                    "wifi_mode"
#define PREF_WIFI_MODE_DESC               "0 -> off\n 1 -> STATION\n 2 -> ACCESS_POINT\n others -> STATION"
#define PREF_WIFI_NAME                    "wifi_name"
String  PREF_WIFI_NAME_DESC =             "default "+String(CONF_WIFI_NAME);
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
  { .key = PREF_PREFIX_WIFI_SSID, .type = STRING, .desc = "" },
  { .key = PREF_PREFIX_WIFI_PSWD, .type = STRING, .desc = "" }
};

const Config config_defs[] = {
  { .key = PREF_REBOOT_FREE, .type = UINT32, .desc = PREF_REBOOT_FREE_DESC },
  { .key = PREF_REBOOT_MS, .type = UINT32, .desc = PREF_REBOOT_MS_DESC },
  { .key = PREF_TIME_ZONE, .type = STRING, .desc = PREF_TIME_ZONE_DESC.c_str() },
#ifdef CONF_WIFI
  { .key = PREF_WIFI_MODE, .type = UINT8, .desc = PREF_WIFI_MODE_DESC },
  { .key = PREF_WIFI_AP_IP, .type = STRING, .desc = PREF_WIFI_AP_IP_DESC.c_str() },
  { .key = PREF_WIFI_AP_SSID, .type = STRING, .desc = PREF_WIFI_AP_SSID_DESC.c_str() },
  { .key = PREF_WIFI_AP_PSWD, .type = STRING, .desc = PREF_WIFI_AP_PSWD_DESC.c_str() },
  { .key = PREF_WIFI_AP_PIN, .type = UINT8, .desc = PREF_WIFI_AP_PIN_DESC },
  { .key = PREF_PREFIX_WIFI, .type = DARRAY, .desc = "", .count = CONF_WIFI_COUNT, .refs = &config_wifi_defs[0], .refs_len = len(config_wifi_defs) },
  { .key = PREF_WIFI_CHECK_INTERVAL, .type = UINT32, .desc = PREF_WIFI_CHECK_INTERVAL_DESC.c_str() },
  { .key = PREF_WIFI_CHECK_THRESHOLD, .type = UINT32, .desc = PREF_WIFI_CHECK_THRESHOLD_DESC.c_str() },
  { .key = PREF_WIFI_NAME, .type = STRING, .desc = PREF_WIFI_NAME_DESC.c_str() },
#endif
#ifdef CONF_WEB
  { .key = PREF_WEB_HTML_TITLE, .type = STRING, .desc = PREF_WEB_HTML_TITLE_DESC.c_str() },
  { .key = PREF_WEB_ADMIN_USERNAME, .type = STRING, .desc = PREF_WEB_ADMIN_USERNAME_DESC.c_str() },
  { .key = PREF_WEB_ADMIN_PASSWORD, .type = STRING, .desc = PREF_WEB_ADMIN_PASSWORD_DESC.c_str() },
  { .key = PREF_WEB_USERS, .type = STRUCT, .desc = "" },
#endif
#ifdef CONF_WEB_HTTPS
  { .key = PREF_WEB_CERT, .type = STRING, .desc = PREF_WEB_CERT_DESC },
  { .key = PREF_WEB_CERT_KEY, .type = STRING, .desc = PREF_WEB_CERT_DESC },
  { .key = PREF_WEB_SECURE, .type = BOOL, .desc = PREF_WEB_SECURE_DESC },
#endif
  { .key = PREF_CONFIG_PUBLISH, .type = BOOL, .desc = "" },
#ifdef CONF_DHT
  { .key = PREF_DHT_PIN, .type = UINT8, .desc = "" },
  { .key = PREF_DHT_TYPE, .type = UINT8, .desc = "" },
  { .key = PREF_DHT_READ_INTERVAL, .type = UINT32, .desc = PREF_DHT_READ_INTERVAL_DESC.c_str() },
#endif
  { .key = PREF_BOILER_PIN, .type = UINT8, .desc = "" },
  { .key = PREF_THERMO_AUTO_INTERVAL, .type = UINT32, .desc = PREF_THERMO_AUTO_INTERVAL_DESC.c_str() },
  { .key = PREF_THERMO_REFRESH, .type = UINT32, .desc = PREF_THERMO_REFRESH_DESC.c_str() },
  { .key = PREF_LOG_LEVEL, .type = STRING, .desc = PREF_LOG_LEVEL_DESC }
};

Preferences preferences;

void preferences_on_update(JSONVar message);

void preferences_on_update(const char * name, String value) {
  JSONVar message;
  message[name] = value;
  preferences_on_update(message);
}

void preferences_on_update(const char * name, int32_t value) {
  JSONVar message;
  message[name] = value;
  preferences_on_update(message);
}

void preferences_on_update(const char * name, uint32_t value) {
  JSONVar message;
  message[name] = value;
  preferences_on_update(message);
}

void preferences_on_update(const char * name, bool value) {
  JSONVar message;
  message[name] = value;
  preferences_on_update(message);
}

void preferences_on_update(const char * name, double value) {
  JSONVar message;
  message[name] = value;
  preferences_on_update(message);
}

String preferences_get(const char * key,ctype_t type,bool emptyOnNull=false) {
  if (emptyOnNull && !preferences.isKey(key)) {
    return "";
  }
  switch (type) {
    case UINT8: return String(preferences.getUChar(key));
    case UINT16: return String(preferences.getUShort(key));
    case UINT32: return String(preferences.getULong(key));
    case UINT64: return uint64_to_string(preferences.getULong64(key));
    case INT8: return String(preferences.getChar(key));
    case INT16: return String(preferences.getShort(key));
    case INT32: return String(preferences.getLong(key));
    case INT64: return int64_to_string((long long) preferences.getLong64(key));
    case STRING: return preferences.getString(key);
    case BOOL: return preferences.getBool(key) ? "true" : "false";
    case FLOAT: return String(preferences.getFloat(key));
    case DOUBLE: return String(preferences.getDouble(key));
    case STRUCT: return preferences.getString(key);
    default: return "";
  }
}

bool preferences_remove(const char * key,ctype_t type,String publish_key = "") {
  log_d("removing %s",key);
  preferences.remove(key);
  if (publish_key != "") {
    if (type == STRING) {
      preferences_on_update(publish_key.c_str(),"");
    } else if (type == BOOL) {
      preferences_on_update(publish_key.c_str(),false);
    } else {
      preferences_on_update(publish_key.c_str(),0);
    }
  }
  return true;
}

bool preferences_put(const char * key,ctype_t type,String value,String publish_key = "") {
  if (type == UINT8) {
    uint8_t number = str_to_uint8(value);
    log_d("saving %s = %hhu",key,number);
    preferences.putUChar(key,number);
    if (publish_key != "") {
      preferences_on_update(publish_key.c_str(),number);
    }
  } else if (type == UINT16) {
    uint16_t number = str_to_uint16(value);
    log_d("saving %s = %" PRIu16,key,number);
    preferences.putUShort(key,number);
    if (publish_key != "") {
      preferences_on_update(publish_key.c_str(),number);
    }
  } else if (type == UINT32) {
    uint32_t number = str_to_uint32(value);
    log_d("saving %s = %" PRIu32,key,number);
    preferences.putULong(key,number);
    if (publish_key != "") {
      preferences_on_update(publish_key.c_str(),number);
    }
  } else if (type == UINT64) {
    uint64_t number = str_to_uint64(value);
    log_d("saving %s = %" PRIu64,key,number);
    preferences.putULong64(key,number);
    if (publish_key != "") {
      preferences_on_update(publish_key.c_str(),uint64_to_string(number));
    }
  } else if (type == INT8) {
    int8_t number = str_to_int8(value);
    log_d("saving %s = %" PRIi8,key,number);
    preferences.putChar(key,number);
    if (publish_key != "") {
      preferences_on_update(publish_key.c_str(),number);
    }
  } else if (type == INT16) {
    int16_t number = str_to_int16(value);
    log_d("saving %s = %" PRIi16,key,number);
    preferences.putShort(key,number);
    if (publish_key != "") {
      preferences_on_update(publish_key.c_str(),number);
    }
  } else if (type == INT32) {
    int32_t number = str_to_int32(value);
    log_d("saving %s = %" PRIi32,key,number);
    preferences.putLong(key,number);
    if (publish_key != "") {
      preferences_on_update(publish_key.c_str(),number);
    }
  } else if (type == INT64) {
    int64_t number = str_to_int64(value);
    log_d("saving %s = %" PRIi64,key,number);
    preferences.putLong64(key,number);
    if (publish_key != "") {
      preferences_on_update(publish_key.c_str(),int64_to_string(number));
    }
  } else if (type == STRING) {
    log_d("saving %s = %s",key,value.c_str());
    preferences.putString(key,value);
    if (publish_key != "") {
      preferences_on_update(publish_key.c_str(),value);
    }
  } else if (type == BOOL) {
    bool number = str_to_bool(value);
    log_d("saving %s = %s",key,value.c_str());
    preferences.putBool(key,number);
    if (publish_key != "") {
      preferences_on_update(publish_key.c_str(),number);
    }
  } else if (type == FLOAT) {
    float number = str_to_float(value);
    log_d("saving %s = %f",key,number);
    preferences.putFloat(key,number);
    if (publish_key != "") {
      preferences_on_update(publish_key.c_str(),number);
    }
  } else if (type == DOUBLE) {
    double number = str_to_double(value);
    log_d("saving %s = %lf",key,number);
    preferences.putDouble(key,number);
    if (publish_key != "") {
      preferences_on_update(publish_key.c_str(),number);
    }
  } else if (type == STRUCT) {
    log_d("saving %s = %s",key,value.c_str());
    preferences.putString(key,value);
    if (publish_key != "") {
      preferences_on_update(publish_key.c_str(),value);
    }
  } else {
    return false;
  }
  return true;
}

#endif

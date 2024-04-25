#ifndef INCLUDE_WIFI_UTILS_H
#define INCLUDE_WIFI_UTILS_H

/**
 * Contiene variabili, tipi e funzioni per l'uso del WiFi.
 * 
 * @see https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFi.h
 * @see https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiMulti.h
 * @see https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/WiFi.h
 * @see https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/ESP8266WiFiMulti.h
 */

#include "base_utils.h"
#ifdef PLATFORM_ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#else
#include <WiFi.h>
#include <WiFiMulti.h>
#endif

class WiFiUtils {
private:
  static uint8_t _mode;
  static uint8_t _mode_setup;
  static IPAddress _ap_ip;
  static IPAddress _ap_subnet;
  static String _ap_ssid;
  static String _ap_pswd;
  static uint8_t _count; 
  static uint32_t _conn_timeout;
  static uint32_t _check_interval_ms;
  static uint32_t _check_threshold_ms;
  static uint32_t _last_check;
  static uint32_t _last_check_ok;
#ifdef PLATFORM_ESP8266
  static ESP8266WiFiMulti _multi;
#else
  static WiFiMulti _multi;
#endif
  static std::function<void(uint8_t,bool)> _state_changed;
public:
  static uint8_t getMode();
  static void setMode(uint8_t mode);
  static bool isEnabled();
  static bool isConnected();
  static String getIP();
  static String getInfo();
  static void loopToHandleConnection(uint32_t mode_limit_ms = 0);
  static void addAP(const char* ssid, const char* pswd);
  static void setup(uint8_t mode, const char* ap_ip, const char* ap_ssid, const char* ap_pswd, 
    uint32_t conn_timeout = CONF_WIFI_CONN_TIMEOUT_MS,
    uint32_t check_interval_ms = CONF_WIFI_CHECK_INTERVAL_MIN, 
    uint32_t check_threshold_ms = CONF_WIFI_CHECK_THRESHOLD,
    std::function<void(uint8_t,bool)> state_changed = NULL);
};

uint8_t WiFiUtils::_mode = WIFI_OFF;
uint8_t WiFiUtils::_mode_setup = WIFI_OFF;
IPAddress WiFiUtils::_ap_ip;
IPAddress WiFiUtils::_ap_subnet = IPAddress(255,255,255,0);
String WiFiUtils::_ap_ssid;
String WiFiUtils::_ap_pswd;
uint8_t WiFiUtils::_count = 0; 
uint32_t WiFiUtils::_conn_timeout;
uint32_t WiFiUtils::_check_interval_ms;
uint32_t WiFiUtils::_check_threshold_ms;
uint32_t WiFiUtils::_last_check = 0;
uint32_t WiFiUtils::_last_check_ok = 0;
#ifdef PLATFORM_ESP8266
ESP8266WiFiMulti WiFiUtils::_multi;
#else
WiFiMulti WiFiUtils::_multi;
#endif
std::function<void(uint8_t,bool)> WiFiUtils::_state_changed = NULL;

uint8_t WiFiUtils::getMode() {
  return _mode;
}

void WiFiUtils::setMode(uint8_t mode) {
  if (mode == WIFI_OFF) {
    WiFi.disconnect(true);
    _mode = mode;
    if (_state_changed != NULL) {
      _state_changed(_mode,false);
    }
    return;
  }
  if (mode == WIFI_AP) {
    log_w("Attivazione AP in corso ...");
    log_d("%s %s",_ap_ssid.c_str(),_ap_pswd.c_str());
    WiFi.softAPConfig(_ap_ip,_ap_ip,_ap_subnet);
    WiFi.softAP(_ap_ssid.c_str(),_ap_pswd.c_str());
  }
  WiFi.mode((WiFiMode_t) mode);
  _mode = mode;
  if (_mode == WIFI_AP) {
    log_w("IP address: %s",getIP().c_str());
  }
  if (_state_changed != NULL) {
    _state_changed(_mode,_mode == WIFI_AP);
  }
}

bool WiFiUtils::isEnabled() {
  return _mode != WIFI_OFF;
}

bool WiFiUtils::isConnected() {
  return _mode == WIFI_STA;
}

String WiFiUtils::getIP() {
  if (_mode == WIFI_STA) {
    //return WiFi.localIPv6().toString();
    return WiFi.localIP().toString();
  }
  if (_mode == WIFI_AP) {
    return WiFi.softAPIP().toString();
  }
  return "";
}

String WiFiUtils::getInfo() {
  if (_mode == WIFI_STA) {
    return "Connected to \""+WiFi.SSID()+"\" (RSSI "+String(WiFi.RSSI())+")";
  } else if (_mode = WIFI_AP) {
    return "Connected clients: "+String(WiFi.softAPgetStationNum());
  } else {
    return "WiFi is OFF";
  }
}

void WiFiUtils::loopToHandleConnection(uint32_t mode_limit_ms) {
  if (_mode != _mode_setup && at_interval(mode_limit_ms)) {
    if (_mode_setup == WIFI_OFF) {
      setMode(WIFI_OFF);
    } else if (_mode_setup == WIFI_AP && _ap_ssid != "") {
      setMode(WIFI_AP);
    } else {
      ESP.restart();
      return;
    }
  }
  if (_mode == WIFI_STA) {
    if (at_interval(_check_interval_ms,_last_check)) {
      _last_check = millis();
      if (_multi.run(_conn_timeout) == WL_CONNECTED) {
        _last_check_ok = _last_check;
        if (_state_changed != NULL) {
          _state_changed(_mode,false);
        }
      } else {
        if (_state_changed != NULL) {
          _state_changed(_mode,true);
        }
        if (_check_threshold_ms == 0 || at_interval(_check_threshold_ms,_last_check_ok)) {
          ESP.restart();
          return;
        }
      } 
    }
  }
}

void WiFiUtils::addAP(const char* ssid, const char* pswd) {
  _multi.addAP(ssid,pswd);
  _count++;
}

void WiFiUtils::setup(uint8_t mode, const char* ap_ip, const char* ap_ssid, const char* ap_pswd, 
    uint32_t conn_timeout, uint32_t check_interval_ms, uint32_t check_threshold_ms,
    std::function<void(uint8_t,bool)> state_changed) {
  log_i("Preparazione WIFI ...");
  _mode_setup = mode;
  _ap_ip.fromString(ap_ip);
  _ap_ssid = String(ap_ssid);
  _ap_pswd = String(ap_pswd);
  _conn_timeout = conn_timeout;
  _check_interval_ms = max(check_interval_ms,CONF_WIFI_CHECK_INTERVAL_MIN);
  _check_threshold_ms = check_threshold_ms;
  _state_changed = state_changed;
  setMode(WIFI_OFF);
  bool connected = false;
  if (_count > 0) {
    setMode(WIFI_STA);
    log_i("Connessione in corso ...");
    if (_multi.run(_conn_timeout) == WL_CONNECTED) {
      log_i("IP address: %s", getIP().c_str());
      connected = true;
    } else {
      log_i("Connessione non riuscita");
    }
  }
  if (!connected) {
    if (_ap_ssid == "") {
      if (_count == 0) {
        setMode(WIFI_OFF);
      }
    } else {
      setMode(WIFI_AP);
    }
  }
}

#endif

#ifndef INCLUDE_WIFI_H
#define INCLUDE_WIFI_H

/**
 * Contiene variabili, tipi e funzioni per l'uso del WiFi.
 * 
 * @see https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiMulti.h
 * @see https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiClient.h
 */

#include "base_utils.h"
#include <WiFi.h>
#include <WiFiMulti.h>

wifi_mode_t wifi_mode;
uint8_t wifi_mode_setup;
IPAddress wifi_ap_ip;
IPAddress wifi_ap_subnet(255,255,255,0);
String wifi_ap_ssid;
String wifi_ap_pswd;
uint8_t wifi_count = 0; 
uint32_t wifi_conn_timeout;
uint32_t wifi_check_interval_ms;
uint32_t wifi_check_threshold_ms;
uint32_t wifi_last_check = 0;
uint32_t wifi_last_check_ok = 0;
WiFiMulti wifiMulti;
WiFiClient wifiClient;

void wifi_set_mode(wifi_mode_t mode) {
  if (mode == WIFI_OFF) {
    WiFi.disconnect(true);
  }
  WiFi.mode(mode);
  wifi_mode = mode;
}

void wifi_ap_state_changed(int value, bool skip_publish = false);

bool wifi_is_off() {
  return wifi_mode == WIFI_OFF;
}

bool wifi_have_internet() {
  return wifi_mode == WIFI_STA;
}

String wifi_get_ip_address() {
  if (wifi_mode == WIFI_STA) {
    //return WiFi.localIPv6().toString();
    return WiFi.localIP().toString();
  }
  if (wifi_mode == WIFI_AP) {
    return WiFi.softAPIP().toString();
  }
  return "";
}

String wifi_get_info() {
  if (wifi_mode = WIFI_STA) {
    return "Connected to \""+WiFi.SSID()+"\" (RSSI "+String(WiFi.RSSI())+")";
  } else if (wifi_mode = WIFI_AP) {
    return "Connected clients: "+String(WiFi.softAPgetStationNum());
  } else {
    return "WiFi is OFF";
  }
}

void wifi_ap_mode() {
  wifi_set_mode(WIFI_OFF);
  log_w("Attivazione AP in corso ...");
  wifi_set_mode(WIFI_AP);
  WiFi.softAPConfig(wifi_ap_ip,wifi_ap_ip,wifi_ap_subnet);
  WiFi.softAP(wifi_ap_ssid.c_str(),wifi_ap_pswd.c_str());
  log_w("IP address: %s", wifi_get_ip_address().c_str());
  wifi_ap_state_changed(HIGH);
}

void wifi_loop(uint32_t mode_limit_ms = 0) {
  if (wifi_mode != wifi_mode_setup && at_interval(mode_limit_ms)) {
    if (wifi_mode_setup == 0) {
      wifi_set_mode(WIFI_OFF);
    } else if (wifi_mode_setup == 2 && wifi_ap_ssid != "") {
      wifi_ap_mode();
    } else {
      ESP.restart();
      return;
    }
  }
  if (wifi_mode == WIFI_STA) {
    if (at_interval(wifi_check_interval_ms,wifi_last_check)) {
      wifi_last_check = millis();
      if (wifiMulti.run(wifi_conn_timeout) == WL_CONNECTED) {
        wifi_last_check_ok = wifi_last_check;
        wifi_ap_state_changed(LOW);
      } else {
        wifi_ap_state_changed(HIGH);
        if (wifi_check_threshold_ms == 0 || at_interval(wifi_check_threshold_ms,wifi_last_check_ok)) {
          ESP.restart();
          return;
        }
      } 
    }
  }
}

void wifi_add_ap(const char * ssid, const char * pswd) {
  wifiMulti.addAP(ssid,pswd);
  wifi_count++;
}

void wifi_setup(uint8_t mode, const char * ap_ip, const char * ap_ssid, const char * ap_pswd, uint32_t conn_timeout, uint32_t check_interval_ms, uint32_t check_threshold_ms) {
  log_i("Preparazione WIFI ...");
  wifi_mode_setup = mode;
  wifi_ap_ip.fromString(ap_ip);
  wifi_ap_ssid = String(ap_ssid);
  wifi_ap_pswd = String(ap_pswd);
  wifi_conn_timeout = conn_timeout;
  wifi_check_interval_ms = check_interval_ms;
  wifi_check_threshold_ms = check_threshold_ms;
  WiFi.disconnect();
  bool connected = false;
  wifi_ap_state_changed(LOW, wifi_count == 0);
  if (wifi_count > 0) {
    wifi_set_mode(WIFI_STA);
    log_i("Connessione in corso ...");
    if (wifiMulti.run(wifi_conn_timeout) == WL_CONNECTED) {
      log_i("IP address: %s", wifi_get_ip_address().c_str());
      connected = true;
    } else {
      log_i("Connessione non riuscita");
    }
  }
  if (!connected) {
    if (wifi_ap_ssid == "") {
      if (wifi_count == 0) {
        wifi_set_mode(WIFI_OFF);
      }
    } else {
      wifi_ap_mode();
    }
  }
}

#endif

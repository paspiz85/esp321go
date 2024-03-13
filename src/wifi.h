#ifndef INCLUDE_WIFI_H
#define INCLUDE_WIFI_H

/**
 * Contiene variabili, tipi e funzioni per l'uso del WiFi.
 * 
 * @see https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiMulti.h
 * @see https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiClient.h
 * @see https://github.com/espressif/arduino-esp32/blob/master/libraries/HTTPClient/src/HTTPClient.h
 */

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

wifi_mode_t wifi_mode;
uint8_t wifi_mode_setup;
String wifi_ap_ip;
String wifi_ap_ssid;
String wifi_ap_pswd;
uint8_t wifi_count = 0; 
uint32_t wifi_conn_timeout;
uint32_t wifi_check_interval;
uint32_t wifi_check_threshold;
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

void wifi_ap_state_changed(int value,bool skip_publish=false);

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

String wifi_info() {
  if (wifi_mode = WIFI_STA) {
    return "Connected to \""+html_encode(WiFi.SSID())+"\" (RSSI "+String(WiFi.RSSI())+")";
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
  IPAddress local_ip;
  local_ip.fromString(wifi_ap_ip.c_str());
  IPAddress gateway;
  gateway.fromString(wifi_ap_ip.c_str());
  IPAddress subnet(255,255,255,0);
  WiFi.softAPConfig(local_ip,gateway,subnet);
  WiFi.softAP(wifi_ap_ssid.c_str(),wifi_ap_pswd.c_str());
  log_w("IP address: %s", wifi_get_ip_address().c_str());
  wifi_ap_state_changed(HIGH);
}

void wifi_loop(uint32_t mode_limit) {
  if (wifi_mode != wifi_mode_setup && millis() > mode_limit) {
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
    if (millis() - wifi_last_check > wifi_check_interval) {
      wifi_last_check = millis();
      if (wifiMulti.run(wifi_conn_timeout) == WL_CONNECTED) {
        wifi_last_check_ok = wifi_last_check;
        wifi_ap_state_changed(LOW);
      } else {
        wifi_ap_state_changed(HIGH);
        if (wifi_check_threshold == 0 || millis() - wifi_last_check_ok > wifi_check_threshold) {
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

void wifi_setup(uint8_t mode, String ap_ip, String ap_ssid, String ap_pswd, uint32_t conn_timeout, uint32_t check_interval, uint32_t check_threshold) {
  log_i("Preparazione WIFI ...");
  wifi_mode_setup = mode;
  wifi_ap_ip = ap_ip;
  wifi_ap_ssid = ap_ssid;
  wifi_ap_pswd = ap_pswd;
  wifi_conn_timeout = conn_timeout;
  wifi_check_interval = check_interval;
  wifi_check_threshold = check_threshold;
  log_d("wifi_mode_setup = %d", wifi_mode_setup);
  log_d("wifi_ap_ip = %s", wifi_ap_ip.c_str());
  log_d("wifi_ap_ssid = %s", wifi_ap_ssid.c_str());
  log_d("wifi_ap_pswd = %s", wifi_ap_pswd.c_str());
  log_d("wifi_conn_timeout = %d", wifi_conn_timeout);
  log_d("wifi_check_interval = %d", wifi_check_interval);
  log_d("wifi_check_threshold = %d", wifi_check_threshold);
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

#ifndef INCLUDE_WIFI_TIME_H
#define INCLUDE_WIFI_TIME_H

/**
 * Contiene variabili, tipi e funzioni per l'uso del NTP tramite WiFi.
 */

#include "wifi_utils.h"
#include <time.h>

class WiFiTimeClass {
public:
  time_t read();
  bool read(struct tm * timeinfo);
  void loop();
  void setup(const char * ntp_server = CONF_WIFI_NTP_SERVER,
    uint32_t ntp_interval_ms = CONF_WIFI_NTP_INTERVAL,
    const char * timezone = CONF_TIME_ZONE);
private:
  struct tm _info;
  uint32_t _interval_ms = 0;
  uint32_t _last_ms = 0;
};

time_t WiFiTimeClass::read() {
  if (_interval_ms == 0 || _last_ms == 0) {
    return 0;
  }
  return mktime(&_info) + (millis() - _last_ms) / 1000;
}

bool WiFiTimeClass::read(struct tm * timeinfo) {
  time_t epoch = read();
  if (epoch == 0) {
    return false;
  }
  localtime_r(&epoch, timeinfo);
  return true;
}

void WiFiTimeClass::loop() {
  if (at_interval(_interval_ms,_last_ms)) {
    if (getLocalTime(&_info)) {
      _last_ms = millis();
    }
  }
}

void WiFiTimeClass::setup(const char * ntp_server, uint32_t ntp_interval_ms, const char * timezone) {
  if (!WiFiUtils.isConnected()) {
    return;
  }
  configTime(0, 0, ntp_server);
  _interval_ms = max(ntp_interval_ms, CONF_WIFI_NTP_INTERVAL_MIN);
  setenv("TZ",timezone,1);
  tzset();
  if (getLocalTime(&_info)){
    _last_ms = millis();
    char buf[80];
    strftime(buf, sizeof(buf), "NTP time: %Y-%m-%d %H:%M:%S zone %Z %z", &_info);
    log_i("%s", buf);
  } else {
    log_e("NTP time: connection failed");
  }
}

WiFiTimeClass WiFiTime;

#endif

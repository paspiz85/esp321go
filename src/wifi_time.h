#ifndef INCLUDE_WIFI_TIME_H
#define INCLUDE_WIFI_TIME_H

/**
 * Contiene variabili, tipi e funzioni per l'uso del NTP tramite WiFi.
 */

#include "wifi.h"

struct tm wifi_time_info;
uint32_t wifi_time_interval = 0;
uint32_t wifi_time_ms = 0;

time_t wifi_time_read() {
  if (wifi_time_interval == 0 || wifi_time_ms == 0) {
    return 0;
  }
  return mktime(&wifi_time_info) + (millis() - wifi_time_ms) / 1000;
}

bool wifi_time_read(struct tm * timeinfo) {
  time_t epoch = wifi_time_read();
  if (epoch == 0) {
    return false;
  }
  localtime_r(&epoch, timeinfo);
  return true;
}

void wifi_time_loop() {
  if (wifi_time_interval != 0 && millis() - wifi_time_ms > wifi_time_interval) {
    if (getLocalTime(&wifi_time_info)) {
      wifi_time_ms = millis();
    }
  }
}

void wifi_time_setup(const char * ntp_server, uint32_t ntp_interval, const char * timezone) {
  if (!wifi_have_internet()) {
    return;
  }
  configTime(0, 0, ntp_server);
  wifi_time_interval = max(ntp_interval, CONF_WIFI_NTP_INTERVAL_MIN);
  setenv("TZ",timezone,1);
  tzset();
  if (getLocalTime(&wifi_time_info)){
    wifi_time_ms = millis();
    char buf[80];
    strftime(buf, sizeof(buf), "NTP time: %Y-%m-%d %H:%M:%S zone %Z %z", &wifi_time_info);
    log_i("%s", buf);
  } else {
    log_e("NTP time: connection failed");
  }
}

#endif

#ifndef INCLUDE_DHT_H
#define INCLUDE_DHT_H

/**
 * Contiene variabili, tipi e funzioni per l'uso del sensore DHT.
 */

#include "base_conf.h"
#include <DHT.h>

// https://github.com/adafruit/DHT-sensor-library/blob/master/DHT.h

DHT* dht = NULL;
float dht_temp = NAN;
float dht_hum = NAN;
float dht_temp_last = NAN;
float dht_hum_last = NAN;
uint32_t dht_read_interval_ms;
uint32_t dht_read_last_ms = 0;

bool dht_available() {
  return dht != NULL;
}

bool dht_read(bool force = false) {
  if (dht_available() && (force || at_interval(dht_read_interval_ms,dht_read_last_ms))) {
    dht_read_last_ms = millis();
    dht->read(force);
    dht_temp_last = dht->readTemperature();
    if (!isnan(dht_temp_last)) {
      dht_temp = dht_temp_last;
    }
    dht_hum_last = dht->readHumidity();
    if (!isnan(dht_hum_last)) {
      dht_hum = dht_hum_last;
    }
    return true;
  }
  return false;
}

float dht_read_temperature(bool force = false) {
  dht_read(force);
  return force ? dht_temp_last : dht_temp;
}

float dht_read_humidity(bool force = false) {
  dht_read(force);
  return force ? dht_hum_last : dht_hum;
}

bool dht_setup(uint8_t pin, uint8_t type, uint32_t read_interval_ms = CONF_DHT_READ_INTERVAL_MIN) {
  if (pin == 0 || type == 0) {
    return false;
  }
  static DHT dht_local(pin,type);
  dht_read_interval_ms = max(read_interval_ms,CONF_DHT_READ_INTERVAL_MIN);
  dht = &dht_local;
  dht->begin();
  return true;
}

#endif

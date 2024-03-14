#ifndef INCLUDE_DHT_H
#define INCLUDE_DHT_H

/**
 * Contiene variabili, tipi e funzioni per l'uso del sensore DHT.
 */

#include "base_conf.h"
#include <DHT.h>

// https://github.com/adafruit/DHT-sensor-library/blob/master/DHT.h

DHT * dht = NULL;
float dht_temp = NAN;
float dht_hum = NAN;
uint32_t dht_read_interval;
uint32_t dht_read_ts = 0;

bool dht_available() {
  return dht != NULL;
}

bool dht_read(bool force = false) {
  if (dht_available() && (force || millis() - dht_read_ts > dht_read_interval)) {
    dht->read(force);
    float temp = dht->readTemperature();
    if (!isnan(temp)) {
      dht_temp = temp;
    }
    temp = dht->readHumidity();
    if (!isnan(temp)) {
      dht_hum = temp;
    }
    dht_read_ts = millis();
    return true;
  }
  return false;
}

float dht_read_temperature(bool force = false) {
  dht_read(force);
  return dht_temp;
}

float dht_read_humidity(bool force = false) {
  dht_read(force);
  return dht_hum;
}

bool dht_setup(uint8_t pin, uint8_t type, uint32_t read_interval = CONF_DHT_READ_INTERVAL_MIN) {
  if (pin == 0 || type == 0) {
    return false;
  }
  static DHT dht_local(pin,type);
  dht_read_interval = max(read_interval,CONF_DHT_READ_INTERVAL_MIN);
  dht = &dht_local;
  dht->begin();
  return true;
}

#endif

#ifndef INCLUDE_BMP280_H
#define INCLUDE_BMP280_H

/**
 * Contiene variabili, tipi e funzioni per l'uso del sensore BMP 280.
 */

#include "base_conf.h"
#include <Adafruit_BMP280.h>

// https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP280.h

typedef struct bmp280_read {
  uint32_t millis = 0;
  float value = NAN;
} bmp280_read_t;

Adafruit_BMP280 * bmp280 = NULL;
bmp280_read_t last_temp;
bmp280_read_t last_pressure;

bool bmp280_available() {
  return bmp280 != NULL;
}

float bmp280_read_check(float x, bmp280_read* last_read, float delta) {
  if (isnan(x)) {
    return x;
  }
  uint32_t t = millis();
  if (!isnan(last_read->value)) {
    uint32_t dt = (t - last_read->millis) / 1000;
    if (dt < 1) {
      dt = 1;
    }
    if (abs((x - last_read->value) / dt) > delta) {
      return NAN;
    }
  }
  last_read->millis = t;
  last_read->value = x;
  return x;
}

float bmp280_read_temperature() {
  if (bmp280 == NULL) {
    return NAN;
  }
  float t = bmp280->readTemperature();
  return bmp280_read_check(t, &last_temp, 1);
}

float bmp280_read_pressure() {
  if (bmp280 == NULL) {
    return NAN;
  }
  float t = bmp280->readPressure();
  return bmp280_read_check(t, &last_pressure, 100);
}

bool bmp280_setup(uint8_t addr) {
  if (addr == 0) {
    return false;
  }
  static Adafruit_BMP280 bmp280_local;
  bool result = bmp280_local.begin(addr);
  if (result) {
    bmp280 = &bmp280_local;
    bmp280_local.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X16,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  }
  return result;
}

#endif

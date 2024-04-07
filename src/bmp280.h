#ifndef INCLUDE_BMP280_H
#define INCLUDE_BMP280_H

/**
 * Contiene variabili, tipi e funzioni per l'uso del sensore BMP 280.
 */

#include "base_conf.h"
#include <Adafruit_BMP280.h>

// https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP280.h

Adafruit_BMP280 * bmp280 = NULL;

bool bmp280_available() {
  return bmp280 != NULL;
}

float bmp280_read_temperature() {
  if (bmp280 == NULL) {
    return NAN;
  }
  float t = bmp280->readTemperature();
  if (t > 70) {
    return NAN;
  }
  return t;
}

float bmp280_read_pressure() {
  if (bmp280 == NULL) {
    return NAN;
  }
  return bmp280->readPressure();
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

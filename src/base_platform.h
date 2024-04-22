#ifndef INCLUDE_BASE_PLATFORM_H
#define INCLUDE_BASE_PLATFORM_H

/**
 * Contiene eventuali macro non presenti sulla piattaforma di riferimento.
 */

#include <Arduino.h>

#ifdef PLATFORM_ESP32
#define HW_ANALOG_READ_MAX    (4095)
#define HW_PIN_COUNT          (40)
#define LED_BUILTIN           (2)
#else
#define HW_ANALOG_READ_MAX    (4095)
#define HW_PIN_COUNT          (10)
#endif

#ifdef PLATFORM_ESP32
uint32_t ESP_getChipId() {
  uint32_t chipId = 0;
  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  return chipId;
}
#else
uint32_t ESP_getChipId() {
  return ESP.getChipId();
}
#endif

#define len_array(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))

#ifdef PLATFORM_ESP8266
#define ESP_LOG_NONE    (0)
#define ESP_LOG_ERROR   (1)
#define ESP_LOG_WARN    (2)
#define ESP_LOG_INFO    (3)
#define ESP_LOG_DEBUG   (4)
#define ESP_LOG_VERBOSE (5)
uint8_t esp_log_level = ESP_LOG_NONE;
#define esp_log_level_set(spec, level) esp_log_level = level
#define log_e(format, ...) if (esp_log_level >= ESP_LOG_ERROR) { Serial.printf(format "\n", ##__VA_ARGS__); }
#define log_w(format, ...) if (esp_log_level >= ESP_LOG_WARN) { Serial.printf(format "\n", ##__VA_ARGS__); }
#define log_i(format, ...) if (esp_log_level >= ESP_LOG_INFO) { Serial.printf(format "\n", ##__VA_ARGS__); }
#define log_d(format, ...) if (esp_log_level >= ESP_LOG_DEBUG) { Serial.printf(format "\n", ##__VA_ARGS__); }
#endif

#endif

#ifndef INCLUDE_BASE_MACROS_H
#define INCLUDE_BASE_MACROS_H

/**
 * Contiene eventuali macro non presenti sulla piattaforma di riferimento.
 */

#include <Arduino.h>

#define len_array(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))

#define ESP_LOG_NONE    (0)
#define ESP_LOG_ERROR   (1)
#define ESP_LOG_WARN    (2)
#define ESP_LOG_INFO    (3)
#define ESP_LOG_DEBUG   (4)
#define ESP_LOG_VERBOSE (5)
uint8_t esp_log_level = ESP_LOG_NONE;
#define esp_log_level_set(spec, level) esp_log_level = level
#define log_e(format, ...) if (esp_log_level >= ESP_LOG_ERROR) { Serial.printf(format, ##__VA_ARGS__);Serial.println(); }
#define log_w(format, ...) if (esp_log_level >= ESP_LOG_WARN) { Serial.printf(format, ##__VA_ARGS__);Serial.println(); }
#define log_i(format, ...) if (esp_log_level >= ESP_LOG_INFO) { Serial.printf(format, ##__VA_ARGS__);Serial.println(); }
#define log_d(format, ...) if (esp_log_level >= ESP_LOG_DEBUG) { Serial.printf(format, ##__VA_ARGS__);Serial.println(); }

// LED_BUILTIN 2

#endif

#ifndef INCLUDE_BASE_PLATFORM_H
#define INCLUDE_BASE_PLATFORM_H

/**
 * Contiene eventuali macro non presenti sulla piattaforma di riferimento.
 */

#include <Arduino.h>

#ifdef PLATFORM_ESP32
#define HW_ANALOG_CHANNEL
#define HW_ANALOG_CHANNEL_COUNT   (16)
#define HW_ANALOG_CHANNEL_FRQ     (5000)
#define HW_ANALOG_CHANNEL_RES     (10)
#define HW_ANALOG_READ_MAX        (4095)
#define HW_ANALOG_WRITE_MAX       ((1<<HW_ANALOG_CHANNEL_RES)-1)
#define HW_PIN_COUNT              (40)
#define LED_BUILTIN               (2)
#else
#define HW_ANALOG_READ_MAX        (1023)
#define HW_ANALOG_WRITE_MAX       (255)
#define HW_PIN_COUNT              (10)
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

#define ARDUHAL_LOG_LEVEL_ERROR      (1)
#define ARDUHAL_LOG_LEVEL_WARN       (2)
#define ARDUHAL_LOG_LEVEL_INFO       (3)
#define ARDUHAL_LOG_LEVEL_DEBUG      (4)
#define ARDUHAL_LOG_LEVEL_VERBOSE    (5)

#ifndef CONFIG_ARDUHAL_LOG_DEFAULT_LEVEL
#define CONFIG_ARDUHAL_LOG_DEFAULT_LEVEL ARDUHAL_LOG_LEVEL_DEBUG
#endif

#ifndef CORE_DEBUG_LEVEL
#define ARDUHAL_LOG_LEVEL CONFIG_ARDUHAL_LOG_DEFAULT_LEVEL
#else
#define ARDUHAL_LOG_LEVEL CORE_DEBUG_LEVEL
#endif

typedef enum {
    ESP_LOG_NONE,       /*!< No log output */
    ESP_LOG_ERROR,      /*!< Critical errors, software module can not recover on its own */
    ESP_LOG_WARN,       /*!< Error conditions from which recovery measures have been taken */
    ESP_LOG_INFO,       /*!< Information messages which describe normal flow of events */
    ESP_LOG_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    ESP_LOG_VERBOSE     /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
} esp_log_level_t;
esp_log_level_t esp_log_level = ESP_LOG_NONE;
#define esp_log_level_set(spec, level) esp_log_level = level

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_ERROR
#define log_e(format, ...) if (esp_log_level >= ESP_LOG_ERROR) { Serial.printf(format "\n", ##__VA_ARGS__); }
#else
#define log_e(format, ...)
#endif

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_WARN
#define log_w(format, ...) if (esp_log_level >= ESP_LOG_WARN) { Serial.printf(format "\n", ##__VA_ARGS__); }
#else
#define log_w(format, ...)
#endif

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
#define log_i(format, ...) if (esp_log_level >= ESP_LOG_INFO) { Serial.printf(format "\n", ##__VA_ARGS__); }
#else
#define log_i(format, ...)
#endif

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
#define log_d(format, ...) if (esp_log_level >= ESP_LOG_DEBUG) { Serial.printf(format "\n", ##__VA_ARGS__); }
#else
#define log_d(format, ...)
#endif

#endif

#endif

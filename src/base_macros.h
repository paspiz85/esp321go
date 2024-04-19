#ifndef INCLUDE_BASE_MACROS_H
#define INCLUDE_BASE_MACROS_H

/**
 * Contiene eventuali macro non presenti sulla piattaforma di riferimento.
 */

#include <Arduino.h>

#define len_array(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))

#define log_e(format, ...) Serial.printf(format, ##__VA_ARGS__);Serial.println()
#define log_w(format, ...) Serial.printf(format, ##__VA_ARGS__);Serial.println()
#define log_i(format, ...) Serial.printf(format, ##__VA_ARGS__);Serial.println()
#define log_d(format, ...) Serial.printf(format, ##__VA_ARGS__);Serial.println()

#endif

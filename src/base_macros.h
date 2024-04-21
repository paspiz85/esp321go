#ifndef INCLUDE_BASE_MACROS_H
#define INCLUDE_BASE_MACROS_H

/**
 * Contiene eventuali macro non presenti sulla piattaforma di riferimento.
 */

#include <Arduino.h>

uint32_t ESP_getChipId() {
  uint32_t chipId = 0;
  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  return chipId;
}

#define len_array(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))

#define LED_BUILTIN (2)

#endif

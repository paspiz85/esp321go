#ifndef INCLUDE_BASE_PLATFORM_H
#define INCLUDE_BASE_PLATFORM_H

/**
 * Contiene eventuali macro non presenti sulla piattaforma di riferimento.
 */

#include <Arduino.h>

#define HW_ANALOG_READ_MAX    (4095)
#define HW_PIN_COUNT          (40)

#define LED_BUILTIN           (2)

uint32_t ESP_getChipId() {
  uint32_t chipId = 0;
  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  return chipId;
}

#define len_array(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))


#endif

#ifndef INCLUDE_BASE_MACROS_H
#define INCLUDE_BASE_MACROS_H

/**
 * Contiene eventuali macro non presenti sulla piattaforma di riferimento.
 */

#include <Arduino.h>

#define len_array(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))

// LED_BUILTIN ??

#endif

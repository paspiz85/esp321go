#ifndef BASE_MACROS_H
#define BASE_MACROS_H

/**
 * Contiene eventuali macro non presenti sulla piattaforma di riferimento.
 */

#define len(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))

#endif

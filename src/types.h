#ifndef INCLUDE_TYPES_H
#define INCLUDE_TYPES_H

#include <Arduino.h>

typedef enum {
  NO_INPUT = 0,
  DIGITAL_INPUT = 1,
  ANALOG_INPUT = 2,
  MQ2 = 3
} input_type_t;

typedef struct input {
  uint8_t num;
  input_type_t type;
  String key;
  String name;
  int8_t pin;
  bool monitored;
  bool published;
} Input;

typedef enum {
  NONE = 0,
  DIGITAL_OUTPUT = 1,
  ANALOG_PWM_OUTPUT = 2,
  ANALOG_FM_OUTPUT = 3,
  PULSE_1_OUTPUT = 5,
  UINT8_VAR = 16,
  UINT16_VAR = 17,
  UINT32_VAR = 18,
  UINT64_VAR = 19,
  INT8_VAR = 20,
  INT16_VAR = 21,
  INT32_VAR = 22,
  INT64_VAR = 23,
  STRING_VAR = 24,
  BOOL_VAR = 25,
  FLOAT_VAR = 26,
  DOUBLE_VAR = 27
} output_type_t;

typedef struct output {
  uint8_t num;
  output_type_t type;
  String key;
  String name;
  int8_t pin;
  bool stored;
  bool published;
} Output;

#endif

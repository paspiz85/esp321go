#ifndef INCLUDE_BASE_MEMORY_H
#define INCLUDE_BASE_MEMORY_H

/**
 * Contiene funzioni per la memorizzazione dello stato dei pin.
 */

#include "base_conf.h"
#include <Arduino.h>

int pin_states[CONF_SCHEMA_PIN_COUNT] = {-1};
uint32_t pin_write_last_ms[CONF_SCHEMA_PIN_COUNT] = {0};

void on_digitalWriteState(uint8_t pin, int value, bool is_init = false);
void on_analogWriteState(uint8_t pin, uint16_t value, bool is_init = false);
void on_toneState(uint8_t pin, uint32_t value, bool is_init = false);

int getPinState(uint8_t pin) {
  return pin_states[pin];
}

void setPinState(uint8_t pin, int value) {
  pin_states[pin] = value;
}

void digitalWriteState(uint8_t pin, int value, bool is_init = false) {
  digitalWrite(pin,value);
  pin_states[pin] = value;
  pin_write_last_ms[pin] = millis();
  on_digitalWriteState(pin,value,is_init);
}

void analogWriteState(uint8_t pin, uint16_t value, bool is_init = false) {
  analogWrite(pin, value);
  pin_states[pin] = value;
  pin_write_last_ms[pin] = millis();
  on_analogWriteState(pin,value,is_init);
}

void toneState(uint8_t pin, uint32_t value, bool is_init = false) {
  tone(pin, value);
  pin_states[pin] = value;
  pin_write_last_ms[pin] = millis();
  on_toneState(pin,value,is_init);
}

uint32_t lastWriteMillis(uint8_t pin) {
  return pin_write_last_ms[pin];
}

#endif

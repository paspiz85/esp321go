#ifndef INCLUDE_BASE_MEMORY_H
#define INCLUDE_BASE_MEMORY_H

/**
 * Contiene funzioni per la memorizzazione dello stato dei pin.
 */

#include "base_conf.h"
#include <Arduino.h>

int pin_states[CONF_SCHEMA_PIN_COUNT] = {-1};
uint32_t pin_write_last_ms[CONF_SCHEMA_PIN_COUNT] = {0};

int8_t pin_channel[CONF_SCHEMA_PIN_COUNT] = {-1};
uint8_t channels_used = 0;

void on_digitalWriteState(uint8_t pin, int value, bool is_init = false);
void on_analogWriteState(uint8_t pin, uint16_t value, bool is_init = false);
void on_toneState(uint8_t pin, uint32_t value, bool is_init = false);

void pinAnalogModeSetup(uint8_t pin, double freq, uint8_t resolution_bits) {
  ledcSetup(channels_used,freq,resolution_bits);
  ledcAttachPin(pin, channels_used);
  pin_channel[pin] = channels_used++;
}

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
  ledcWrite(pin_channel[pin], value);
  pin_states[pin] = value;
  pin_write_last_ms[pin] = millis();
  on_analogWriteState(pin,value,is_init);
}

void toneState(uint8_t pin, uint32_t value, bool is_init = false) {
  ledcWriteTone(pin_channel[pin], value);
  pin_states[pin] = value;
  pin_write_last_ms[pin] = millis();
  on_toneState(pin,value,is_init);
}

uint32_t lastWriteMillis(uint8_t pin) {
  return pin_write_last_ms[pin];
}

#endif

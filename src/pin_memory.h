#ifndef INCLUDE_BASE_MEMORY_H
#define INCLUDE_BASE_MEMORY_H

/**
 * Contiene funzioni per la memorizzazione dello stato dei pin.
 */

#include "base_conf.h"
#include <Arduino.h>

class PinMemoryClass {
public:
  int getPinState(uint8_t pin);
  void setPinState(uint8_t pin, int value);
  void digitalWrite(uint8_t pin, int value, bool is_init = false);
  void analogWrite(uint8_t pin, uint16_t value, bool is_init = false);
  void tone(uint8_t pin, uint32_t value, bool is_init = false);
  uint32_t lastWriteMillis(uint8_t pin);
  void setup(void (*on_digitalWrite)(uint8_t,int,bool),
    void (*on_analogWrite)(uint8_t,uint16_t,bool),
    void (*on_tone)(uint8_t,uint32_t,bool));
private:
  int _pin_states[CONF_SCHEMA_PIN_COUNT] = {-1};
  uint32_t _pin_write_last_ms[CONF_SCHEMA_PIN_COUNT] = {0};
  void (*_on_digitalWrite)(uint8_t,int,bool) = nullptr;
  void (*_on_analogWrite)(uint8_t,uint16_t,bool) = nullptr;
  void (*_on_tone)(uint8_t,uint32_t,bool) = nullptr;
};

int PinMemoryClass::getPinState(uint8_t pin) {
  return _pin_states[pin];
}

void PinMemoryClass::setPinState(uint8_t pin, int value) {
  _pin_states[pin] = value;
}

void PinMemoryClass::digitalWrite(uint8_t pin, int value, bool is_init) {
  digitalWrite(pin,value);
  _pin_states[pin] = value;
  _pin_write_last_ms[pin] = millis();
  if (_on_digitalWrite != nullptr) {
    _on_digitalWrite(pin,value,is_init);
  }
}

void PinMemoryClass::analogWrite(uint8_t pin, uint16_t value, bool is_init) {
  analogWrite(pin, value);
  _pin_states[pin] = value;
  _pin_write_last_ms[pin] = millis();
  if (_on_analogWrite != nullptr) {
    _on_analogWrite(pin,value,is_init);
  }
}

void PinMemoryClass::tone(uint8_t pin, uint32_t value, bool is_init) {
  tone(pin, value);
  _pin_states[pin] = value;
  _pin_write_last_ms[pin] = millis();
  if (_on_tone != nullptr) {
    _on_tone(pin,value,is_init);
  }
}

uint32_t PinMemoryClass::lastWriteMillis(uint8_t pin) {
  return _pin_write_last_ms[pin];
}

void PinMemoryClass::setup(void (*on_digitalWrite)(uint8_t,int,bool),
    void (*on_analogWrite)(uint8_t,uint16_t,bool),
    void (*on_tone)(uint8_t,uint32_t,bool)) {
  _on_digitalWrite = on_digitalWrite;
  _on_analogWrite = on_analogWrite;
  _on_tone = on_tone;
}

PinMemoryClass PinMemory;

#endif

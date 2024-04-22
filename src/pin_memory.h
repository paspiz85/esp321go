#ifndef INCLUDE_PIN_MEMORY_H
#define INCLUDE_PIN_MEMORY_H

/**
 * Contiene funzioni per la memorizzazione dello stato dei pin.
 */

#include "base_platform.h"
#include <Arduino.h>

class PinMemoryClass {
public:
#ifdef PLATFORM_ESP32
  void pinAnalogModeSetup(uint8_t pin, double freq, uint8_t resolution_bits);
#endif
  int getPinState(uint8_t pin);
  void setPinState(uint8_t pin, int value);
  void writeDigital(uint8_t pin, int value, bool is_init = false);
  void writePWM(uint8_t pin, uint16_t value, bool is_init = false);
  void writeFM(uint8_t pin, uint32_t value, bool is_init = false);
  uint32_t lastWriteMillis(uint8_t pin);
  void setup(void (*on_digitalWrite)(uint8_t,int,bool),
    void (*on_analogWrite)(uint8_t,uint16_t,bool),
    void (*on_tone)(uint8_t,uint32_t,bool));
private:
  int _pin_states[HW_PIN_COUNT] = {-1};
  uint32_t _pin_write_last_ms[HW_PIN_COUNT] = {0};
#ifdef PLATFORM_ESP32
  int8_t _pin_channel[HW_PIN_COUNT] = {-1};
  uint8_t _channels_used = 0;
#endif
  void (*_on_digitalWrite)(uint8_t,int,bool) = nullptr;
  void (*_on_analogWrite)(uint8_t,uint16_t,bool) = nullptr;
  void (*_on_tone)(uint8_t,uint32_t,bool) = nullptr;
};

#ifdef PLATFORM_ESP32
void PinMemoryClass::pinAnalogModeSetup(uint8_t pin, double freq, uint8_t resolution_bits) {
  ledcSetup(_channels_used, freq, resolution_bits);
  ledcAttachPin(pin, _channels_used);
  _pin_channel[pin] = _channels_used++;
}
#endif

int PinMemoryClass::getPinState(uint8_t pin) {
  return _pin_states[pin];
}

void PinMemoryClass::setPinState(uint8_t pin, int value) {
  _pin_states[pin] = value;
}

void PinMemoryClass::writeDigital(uint8_t pin, int value, bool is_init) {
  digitalWrite(pin,value);
  _pin_states[pin] = value;
  _pin_write_last_ms[pin] = millis();
  if (_on_digitalWrite != nullptr) {
    _on_digitalWrite(pin,value,is_init);
  }
}

void PinMemoryClass::writePWM(uint8_t pin, uint16_t value, bool is_init) {
#ifdef PLATFORM_ESP32
  ledcWrite(_pin_channel[pin], value);
#else
  analogWrite(pin, value);
#endif
  _pin_states[pin] = value;
  _pin_write_last_ms[pin] = millis();
  if (_on_analogWrite != nullptr) {
    _on_analogWrite(pin,value,is_init);
  }
}

void PinMemoryClass::writeFM(uint8_t pin, uint32_t value, bool is_init) {
#ifdef PLATFORM_ESP32
  ledcWriteTone(_pin_channel[pin], value);
#else
  tone(pin, value);
#endif
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

#pragma once

/**
 * Contiene funzioni per la memorizzazione dello stato dei pin.
 */

#include "base_platform.h"
#include <Arduino.h>

class PinMemoryClass {
private:
  int _pin_states[HW_PIN_COUNT] = {-1};
  uint32_t _pin_write_last_ms[HW_PIN_COUNT] = {0};
#ifdef HW_ANALOG_CHANNEL
  int8_t _pin_analog_channel[HW_PIN_COUNT] = {-1};
  uint8_t _analog_channels_used = 0;
#endif
  void (*_on_digitalWrite)(uint8_t,int,bool) = NULL;
  void (*_on_analogWrite)(uint8_t,uint16_t,bool) = NULL;
  void (*_on_tone)(uint8_t,uint32_t,bool) = NULL;
public:
#ifdef HW_ANALOG_CHANNEL
  void pinModeAnalog(uint8_t pin, double freq = HW_ANALOG_CHANNEL_FRQ);
#else
  void pinModeAnalog(uint8_t pin, double freq = 0);
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
};

void PinMemoryClass::pinModeAnalog(uint8_t pin, double freq) {
#ifdef HW_ANALOG_CHANNEL
  ledcSetup(_analog_channels_used, freq, HW_ANALOG_CHANNEL_RES);
  ledcAttachPin(pin, _analog_channels_used);
  _pin_analog_channel[pin] = _analog_channels_used++;
#else
  pinMode(pin,OUTPUT);
#endif
}

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
  if (_on_digitalWrite != NULL) {
    _on_digitalWrite(pin,value,is_init);
  }
}

void PinMemoryClass::writePWM(uint8_t pin, uint16_t value, bool is_init) {
#ifdef HW_ANALOG_CHANNEL
  ledcWrite(_pin_analog_channel[pin], value);
#else
  analogWrite(pin, value);
#endif
  _pin_states[pin] = value;
  _pin_write_last_ms[pin] = millis();
  if (_on_analogWrite != NULL) {
    _on_analogWrite(pin,value,is_init);
  }
}

void PinMemoryClass::writeFM(uint8_t pin, uint32_t value, bool is_init) {
#ifdef HW_ANALOG_CHANNEL
  ledcWriteTone(_pin_analog_channel[pin], value);
#else
  tone(pin, value);
#endif
  _pin_states[pin] = value;
  _pin_write_last_ms[pin] = millis();
  if (_on_tone != NULL) {
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

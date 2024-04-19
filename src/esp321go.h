
#include "base_conf.h"

#ifdef CONF_WIFI
#include "wifi.h"
#include "wifi_time.h"
#endif






void setup() {
  Serial.begin(115200);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);




#ifdef CONF_WIFI
#ifdef CONF_WIFI_AP_PIN
  pinMode(CONF_WIFI_AP_PIN, OUTPUT);
#endif
#endif
}

// the loop function runs over and over again forever
void loop() {
  Serial.println("UP");
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);         
  Serial.println("DOWN");              // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
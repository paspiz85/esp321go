
#include "wifi.h"
#include "wifi_time.h"
#include "config.h"
#include "dht.h"
#include "web.h"
#include "web_pages.h"
#include "web_ota.h"
#include "web_config.h"

uint32_t reboot_free;
uint32_t reboot_ms;

int pin_analog[CONF_SCHEMA_PIN_COUNT];
int pin_digits[CONF_SCHEMA_PIN_COUNT];
int8_t pin_channel[CONF_SCHEMA_PIN_COUNT];
uint8_t wifi_ap_pin = 0;

String html_title;

void items_publish(JSONVar message) {
  if (!wifi_have_internet()) {
    return;
  }
  // TODO non pubblichiamo nulla
}

void pinInit(int pin,int mode) {
  pinMode(pin,mode);
  pin_analog[pin] = -1;
  pin_digits[pin] = -1;
}

String digitalString(int value) {
  return value == HIGH ? "HIGH" : "LOW";
}

void digitalWriteState(uint8_t pin,int value,bool skip_publish=false) {
  digitalWrite(pin,value);
  pin_digits[pin] = value;
}

void analogWriteState(uint8_t pin,uint16_t value,bool skip_publish=false) {
  ledcWrite(pin_channel[pin], value);
  pin_analog[pin] = value;
}

void wifi_ap_state_changed(int value, bool skip_publish) {
  if (wifi_ap_pin != 0 && pin_digits[wifi_ap_pin] != value) {
    digitalWriteState(wifi_ap_pin, value, skip_publish);
  }
}

String web_html_title() {
  return html_title;
}

String web_html_footer(bool admin) {
  String html = "<div>";
  if (wifi_mode = WIFI_STA) {
    html += "Connected to \""+html_encode(WiFi.SSID())+"\" (RSSI "+String(WiFi.RSSI())+") - ";
  } else if (wifi_mode = WIFI_AP) {
    html += "Connected clients: "+String(WiFi.softAPgetStationNum())+" - ";
  } else {
    html += "WiFi is OFF - ";
  }
  html += "Memory Free: " +String(ESP.getFreeHeap());
  html += " - Uptime: " +String(millis()) + "</div>";
  html += "<div style=\"margin-top:1rem\">" + String(COMPILE_VERSION)+" [" + String(__TIMESTAMP__)+"]";
  if (admin) {
    html += " <button class=\"btn btn-secondary\" onclick=\"location='"+String(CONF_WEB_URI_FIRMWARE_UPDATE)+"'\">Update</button>";
  }
  html += "</div>";
  return html;
}

void loop() {
  if (reboot_free > 0 && ESP.getFreeHeap() < reboot_free) {
    ESP.restart();
    return;
  }
  if (reboot_ms > 0 && millis() > reboot_ms) {
    ESP.restart();
    return;
  }
  wifi_loop(CONF_WIFI_MODE_LIMIT);
  wifi_time_loop();
  if (!wifi_is_off()) {
    web_server_loop();
    delay(10);
  } else {
    delay(1000);
  }
}

void web_handle_root() {
  int refresh = web_parameter("refresh").toInt();
  String html = "<body style=\"text-align:center\"><h1>"+html_title+"</h1>";
  html += "Hello";
  html += "</body>";
  web_send_page(html_title,html,refresh);
}

void setup() {
  Serial.begin(CONF_MONITOR_BAUD_RATE);
  while (! Serial);
  uint8_t channel = 0;
  preferences.begin("my-app", false);
  String log_level = preferences.getString(PREF_LOG_LEVEL,CONF_LOG_LEVEL);
  Serial.println("Log level : " + log_level);
  if (log_level == "d") {
    esp_log_level_set("*", ESP_LOG_DEBUG);
  } else if (log_level == "i") {
    esp_log_level_set("*", ESP_LOG_INFO);
  } else if (log_level == "w") {
    esp_log_level_set("*", ESP_LOG_WARN);
  } else {
    esp_log_level_set("*", ESP_LOG_ERROR);
  }
  reboot_free = preferences.getULong(PREF_REBOOT_FREE);
  reboot_ms = preferences.getULong(PREF_REBOOT_MS);
  dht_setup(preferences.getUChar(PREF_DHT_PIN),
    preferences.getUChar(PREF_DHT_TYPE), 
    preferences.getULong(PREF_DHT_READ_INTERVAL));
  wifi_ap_pin = preferences.getUChar(PREF_WIFI_AP_PIN);
  if (wifi_ap_pin != 0) {
    pinInit(wifi_ap_pin, OUTPUT);
  }
  for (uint8_t i = 1; i <= CONF_WIFI_COUNT; i++) {
    String ssid = preferences.getString((PREF_PREFIX_WIFI+String(i)+PREF_PREFIX_WIFI_SSID).c_str());
    String pswd = preferences.getString((PREF_PREFIX_WIFI+String(i)+PREF_PREFIX_WIFI_PSWD).c_str());
    if (ssid != "") {
      wifi_add_ap(ssid.c_str(),pswd.c_str());
    }
  }
  uint32_t wifi_check_interval = preferences.getULong(PREF_WIFI_CHECK_INTERVAL,CONF_WIFI_CHECK_INTERVAL);
  if (wifi_check_interval < CONF_WIFI_CHECK_INTERVAL_MIN) {
    wifi_check_interval = CONF_WIFI_CHECK_INTERVAL_MIN;
  }
  wifi_setup(
    preferences.getUChar(PREF_WIFI_MODE,CONF_WIFI_MODE),
    preferences.getString(PREF_WIFI_AP_IP,CONF_WIFI_AP_IP),
    preferences.getString(PREF_WIFI_AP_SSID,CONF_WIFI_AP_SSID),
    preferences.getString(PREF_WIFI_AP_PSWD,CONF_WIFI_AP_PSWD),
    CONF_WIFI_CONN_TIMEOUT_MS,
    wifi_check_interval,
    preferences.getULong(PREF_WIFI_CHECK_THRESHOLD,CONF_WIFI_CHECK_THRESHOLD)
  );
  delay(1000);
  wifi_time_setup(CONF_NTP_SERVER, CONF_NTP_INTERVAL, preferences.getString(PREF_TIME_ZONE,CONF_TIME_ZONE).c_str());
  html_title = preferences.getString(PREF_HTML_TITLE);
  if (html_title == "") {
    html_title = CONF_HTML_TITLE;
  }
  web_server_setup_http();
  web_admin_setup();
  web_ota_setup();
  web_config_setup();
  web_server_register(HTTP_ANY, "/", web_handle_root);
  web_server_begin(preferences.getString(PREF_WIFI_NAME,CONF_WIFI_NAME));
}

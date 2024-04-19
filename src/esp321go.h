
#include "base_memory.h"
#include "config.h"
#ifdef CONF_DHT
#include "dht.h"
#endif
#ifdef CONF_BMP280
#include "bmp280.h"
#endif
#ifdef CONF_WIFI
#include "wifi.h"
#include "wifi_time.h"
#endif
#ifdef CONF_WEB
#include "web.h"
#include "web_templates.h"
#include "web_ota.h"
#include "web_config.h"
#endif

uint32_t reboot_free;
uint32_t reboot_ms;

uint8_t wifi_ap_pin = 0;

String html_title;

void items_publish(JSONVar message) {
#ifdef CONF_WIFI
  if (!wifi_have_internet()) {
    return;
  }
  // TODO
#endif
}

String digitalString(int value) {
  return value == HIGH ? "HIGH" : "LOW";
}

void on_digitalWriteState(uint8_t pin, int value, bool is_init) {}
void on_analogWriteState(uint8_t pin, uint16_t value, bool is_init) {}
void on_toneState(uint8_t pin, uint32_t value, bool is_init) {}

#ifdef CONF_WIFI
void wifi_ap_state_changed(int value, bool skip_publish) {
  if (wifi_ap_pin != 0 && getPinState(wifi_ap_pin) != value) {
    digitalWriteState(wifi_ap_pin, value, skip_publish);
  }
}
#endif

#ifdef CONF_WEB
String web_html_title() {
  return html_title;
}

String web_html_footer(bool admin) {
  String html = "<div>";
  html += html_encode(wifi_get_info());
  html += " - ";
  html += "Memory Free: " +String(ESP.getFreeHeap());
  html += " - Uptime: " +String(millis()) + "</div>";
  html += "<div style=\"margin-top:1rem\">" + String(COMPILE_VERSION)+" [" + String(__TIMESTAMP__)+"]";
  if (admin) {
    html += " <button class=\"btn btn-secondary\" onclick=\"location='"+String(CONF_WEB_URI_FIRMWARE_UPDATE)+"'\">Update</button>";
  }
  html += "</div>";
  return html;
}

void web_handle_root() {
  int refresh = web_parameter("refresh").toInt();
  String html = "<body style=\"text-align:center\"><h1>"+html_title+"</h1>";
  html += "Hello";
  html += "</body>";
  web_send_page(html_title,html,refresh);
}
#endif

void loop() {
  if (reboot_free > 0 && ESP.getFreeHeap() < reboot_free) {
    ESP.restart();
    return;
  }
  if (at_interval(reboot_ms)) {
    ESP.restart();
    return;
  }
#ifdef CONF_WIFI
  wifi_loop(CONF_WIFI_MODE_LIMIT);
  wifi_time_loop();
#endif
#ifdef CONF_WEB
  if (!wifi_is_off()) {
    web_server_loop();
    delay(10);
  } else {
    delay(1000);
  }
#endif
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
#ifdef CONF_DHT
  dht_setup(preferences.getUChar(PREF_DHT_PIN),
    preferences.getUChar(PREF_DHT_TYPE), 
    preferences.getULong(PREF_DHT_READ_INTERVAL));
#endif
#ifdef CONF_BMP280
  bmp280_setup(preferences.getUChar(PREF_BMP280_ADDR));
#endif
#ifdef CONF_WIFI
  wifi_ap_pin = preferences.getUChar(PREF_WIFI_AP_PIN);
  if (wifi_ap_pin != 0) {
    pinMode(wifi_ap_pin, OUTPUT);
  }
  for (uint8_t i = 1; i <= CONF_WIFI_COUNT; i++) {
    String ssid = preferences.getString((PREF_PREFIX_WIFI+String(i)+PREF_PREFIX_WIFI_SSID).c_str());
    String pswd = preferences.getString((PREF_PREFIX_WIFI+String(i)+PREF_PREFIX_WIFI_PSWD).c_str());
    if (ssid != "") {
      wifi_add_ap(ssid.c_str(),pswd.c_str());
    }
  }
  wifi_setup(
    preferences.getUChar(PREF_WIFI_MODE,CONF_WIFI_MODE),
    preferences.getString(PREF_WIFI_AP_IP,CONF_WIFI_AP_IP).c_str(),
    preferences.getString(PREF_WIFI_AP_SSID,CONF_WIFI_AP_SSID).c_str(),
    preferences.getString(PREF_WIFI_AP_PSWD,CONF_WIFI_AP_PSWD).c_str(),
    CONF_WIFI_CONN_TIMEOUT_MS,
    max(preferences.getULong(PREF_WIFI_CHECK_INTERVAL),CONF_WIFI_CHECK_INTERVAL_MIN),
    preferences.getULong(PREF_WIFI_CHECK_THRESHOLD,CONF_WIFI_CHECK_THRESHOLD)
  );
  delay(1000);
  wifi_time_setup(CONF_WIFI_NTP_SERVER, CONF_WIFI_NTP_INTERVAL, preferences.getString(PREF_TIME_ZONE,CONF_TIME_ZONE).c_str());
#endif
#ifdef CONF_WEB
  html_title = preferences.getString(PREF_WEB_HTML_TITLE);
  if (html_title == "") {
    html_title = CONF_WEB_HTML_TITLE;
  }
  web_server_setup_http();
  web_admin_setup(
    preferences.getString(PREF_WEB_ADMIN_USERNAME,CONF_WEB_ADMIN_USERNAME).c_str(),
    preferences.getString(PREF_WEB_ADMIN_PASSWORD,CONF_WEB_ADMIN_PASSWORD).c_str()
  );
  web_ota_setup();
  web_config_setup(preferences.getBool(PREF_CONFIG_PUBLISH));
  web_server_register(HTTP_ANY, "/", web_handle_root);
  web_server_begin(preferences.getString(PREF_WIFI_NAME,CONF_WIFI_NAME).c_str());
#endif
}

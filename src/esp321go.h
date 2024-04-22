
#include "config.h"
#include "pin_memory.h"
#ifdef CONF_DHT
#include "dht.h"
#endif
#ifdef CONF_BMP280
#include "bmp280.h"
#endif
#ifdef CONF_WIFI
#include "wifi_time.h"
#endif
#ifdef CONF_WEB
#include "web.h"
#include "web_templates.h"
#include "web_config.h"
#include "web_ota.h"
#endif
#ifdef CONF_ARDUINO_OTA
#include <ArduinoOTA.h>
#endif

uint32_t reboot_free;
uint32_t reboot_ms;
String admin_username;
String admin_password;

void items_publish(JSONVar message) {
#ifdef CONF_WIFI
  if (!WiFiUtils.isConnected()) {
    return;
  }
  // TODO
#endif
}

String digitalString(int value) {
  return value == HIGH ? "HIGH" : "LOW";
}

bool blink_led_enabled = true;
int blink_led_pin;
uint32_t blink_last_ms = 0;

#ifdef CONF_WIFI
uint8_t wifi_ap_pin = 0;
#endif

#ifdef CONF_WEB
bool web_admin_authenticate() {
  return Web.authenticate(admin_username.c_str(), admin_password.c_str());
}

String web_html_footer(bool admin) {
  String html = "<hr/><div>";
  html += html_encode(WiFiUtils.getInfo());
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
  int blink = Web.getParameter("blink").toInt();
  switch (blink) {
  case 0: break;
  case 2:
    blink_led_enabled = false;
    digitalWrite(blink_led_pin, LOW);
    Web.sendRedirect("/");
    return;
  case 3:
    blink_led_enabled = false;
    digitalWrite(blink_led_pin, HIGH);
    Web.sendRedirect("/");
    return;
  default:
    blink_led_enabled = true;
    Web.sendRedirect("/");
    return;
  }
  String title = WebTemplates.getTitle();
  String html = "<body style=\"text-align:center\"><h1>"+title+"</h1>";
  html += "<h3>Hello World</h3>";
  html += "<form action=\"/\" method=\"post\">";
  html += "<select name=\"blink\" required>";
  html += "<option value=\"0\"></option>";
  html += "<option value=\"1\">Lampeggia</option>";
  html += "<option value=\"2\">Sempre acceso</option>";
  html += "<option value=\"3\">Sempre spento</option>";
  html += "</select>";
  html += "<p><input type=\"submit\" value=\"OK\" /></p>";
  html += "</form>";
  html += "<p><a href=\"/config\">Configurazione</a></p>";
  html += WebTemplates.getFooter(false);
  html += "</body>";
  WebTemplates.sendPage(title,html);
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
  WiFiUtils.loopToHandleConnection(CONF_WIFI_MODE_LIMIT);
  WiFiTime::loopToSynchronize();
#endif
#ifdef CONF_ARDUINO_OTA
  ArduinoOTA.handle();
#endif
  if (blink_led_enabled && at_interval(1000,blink_last_ms)) {
    blink_last_ms = millis();
    digitalWrite(blink_led_pin, !digitalRead(blink_led_pin));
  }
#ifdef CONF_WEB
  if (WiFiUtils.isEnabled()) {
    Web.loopToHandleClients();
    delay(10);
  } else {
    delay(1000);
  }
#endif
}

void setup() {
  Serial.begin(CONF_MONITOR_BAUD_RATE);
  while (! Serial);
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
  PinMemory.setup([](uint8_t pin, int value, bool is_init) {
  }, [](uint8_t pin, uint16_t value, bool is_init) {
  }, [](uint8_t pin, uint32_t value, bool is_init) {
  });
#ifdef CONF_DHT
  dht_setup(preferences.getUChar(PREF_DHT_PIN),
    preferences.getUChar(PREF_DHT_TYPE), 
    preferences.getULong(PREF_DHT_READ_INTERVAL));
#endif
#ifdef CONF_BMP280
  bmp280_setup(preferences.getUChar(PREF_BMP280_ADDR));
#endif
  blink_led_pin = preferences.getInt(PREF_BLINK_LED_PIN,LED_BUILTIN);
  pinMode(blink_led_pin, OUTPUT);
#ifdef CONF_WIFI
  wifi_ap_pin = preferences.getUChar(PREF_WIFI_AP_PIN);
  if (wifi_ap_pin != 0) {
    pinMode(wifi_ap_pin, OUTPUT);
  }
  for (uint8_t i = 1; i <= CONF_WIFI_COUNT; i++) {
    String ssid = preferences.getString((PREF_PREFIX_WIFI+String(i)+PREF_PREFIX_WIFI_SSID).c_str());
    String pswd = preferences.getString((PREF_PREFIX_WIFI+String(i)+PREF_PREFIX_WIFI_PSWD).c_str());
    if (ssid != "") {
      WiFiUtils.addAP(ssid.c_str(),pswd.c_str());
    }
  }
  WiFiUtils.setup(
    preferences.getUChar(PREF_WIFI_MODE,CONF_WIFI_MODE),
    preferences.getString(PREF_WIFI_AP_IP,CONF_WIFI_AP_IP).c_str(),
    preferences.getString(PREF_WIFI_AP_SSID,CONF_WIFI_AP_SSID).c_str(),
    preferences.getString(PREF_WIFI_AP_PSWD,CONF_WIFI_AP_PSWD).c_str(),
    CONF_WIFI_CONN_TIMEOUT_MS,
    preferences.getULong(PREF_WIFI_CHECK_INTERVAL,CONF_WIFI_CHECK_INTERVAL_MIN),
    preferences.getULong(PREF_WIFI_CHECK_THRESHOLD,CONF_WIFI_CHECK_THRESHOLD),
    [](uint8_t mode, bool connected) {
      if (wifi_ap_pin == 0) {
        return;
      }
      int value = mode == WIFI_AP ? HIGH : LOW;
      if (PinMemory.getPinState(wifi_ap_pin) != value) {
        PinMemory.writeDigital(wifi_ap_pin, value, true);
      }
    }
  );
  delay(1000);
  WiFiTime::setup(CONF_WIFI_NTP_SERVER, CONF_WIFI_NTP_INTERVAL, preferences.getString(PREF_TIME_ZONE,CONF_TIME_ZONE).c_str());
#endif
  admin_username = preferences.getString(PREF_ADMIN_USERNAME,CONF_ADMIN_USERNAME);
  admin_password = preferences.getString(PREF_ADMIN_PASSWORD,CONF_ADMIN_PASSWORD);
#ifdef CONF_ARDUINO_OTA
  ArduinoOTA.setPassword(admin_password.c_str());
  ArduinoOTA.begin();
#endif
#ifdef CONF_WEB
  String web_html_title = preferences.getString(PREF_WEB_HTML_TITLE);
  if (web_html_title == "") {
    web_html_title = CONF_WEB_HTML_TITLE;
  }
  Web.setupHTTP();
#ifdef CONF_WEB_HTTPS
  Web.setupHTTPS(preferences.getString(PREF_WEB_CERT),preferences.getString(PREF_WEB_CERT_KEY));
#endif
  WebTemplates.setup(web_html_title, web_html_footer);
  web_config_setup(web_admin_authenticate,preferences.getBool(PREF_CONFIG_PUBLISH));
  web_ota_setup(web_admin_authenticate);
  Web.handle(HTTP_ANY, "/", web_handle_root);
  Web.begin(preferences.getString(PREF_WIFI_NAME,CONF_WIFI_NAME).c_str());
#endif
}
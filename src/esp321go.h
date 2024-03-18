
#include "base_memory.h"
#include "config.h"
#ifdef CONF_DHT
#include "dht.h"
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
#include "web_users.h"
#endif

#define THERMO_MODE_OFF     (0)
#define THERMO_MODE_AUTO    (1)
#define THERMO_MODE_BOOST30 (3)
#define THERMO_MODE_BOOST60 (4)
#define THERMO_MODE_BOOST90 (5)
#define THERMO_TARGET_MIN   (7)
#define THERMO_TARGET_MAX   (30)
#define THERMO_TARGET_DEF   (15)

uint32_t reboot_free;
uint32_t reboot_ms;

uint8_t wifi_ap_pin = 0;
uint8_t boiler_pin = 0;

String html_title;
uint8_t thermo_mode;
float thermo_target;
uint32_t thermo_auto_interval;
uint32_t thermo_auto_last = 0;
uint32_t thermo_auto_fail = 0;
uint32_t thermo_boost_stop = 0;
uint8_t thermo_boost_reset;
String thermo_boost_begin;
String thermo_boost_end;
uint16_t thermo_refresh;

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

void boiler_write(bool b) {
  if (boiler_pin != 0) {
    if (b) {
      if (pin_states[boiler_pin] == 0) {
        digitalWriteState(boiler_pin, HIGH);
      }
    } else {
      if (pin_states[boiler_pin] != 0) {
        digitalWriteState(boiler_pin, LOW);
      }
    }
  }
}

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
  html += wifi_get_info();
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

JSONVar html_data() {
  JSONVar data;
  struct tm timeinfo;
  if (wifi_time_read(&timeinfo)){
    char time_str[100];
    strftime(time_str,sizeof(time_str),"%H:%M:%S - %A %e %B %Y",&timeinfo);
    data["time_ref"] = String(time_str);
  } else {
    data["time_ref"] = "Temperatura attuale";
  }
  float temp = dht_read_temperature();
  if (isnan(temp)) {
    data["temp"] = "--";
  } else {
    data["temp"] = String(temp,1) + " &deg;C";
  }
  temp = dht_read_humidity();
  if (isnan(temp)) {
    data["hum"] = "--";
  } else {
    data["hum"] = String(temp,0) + "%";
  }
  if (boiler_pin == 0) {
    data["boiler_status"] = "--";
  } else {
    if (pin_states[boiler_pin] == 0) {
      data["boiler_status"] = "OFF";
    } else {
      data["boiler_status"] = "ON";
    }
  }
  return data;
}

void web_handle_rest_read(HTTPRequest * req, HTTPResponse * res) {
  return web_send_data(req,res,html_data());
}

void web_handle_root(HTTPRequest * req, HTTPResponse * res) {
  if (!web_login(req,res)) {
    return;
  }
  uint8_t mode = 0;
  float target = 0;
  HTTPBodyParser *parser = NULL;
  if (req->getHeader("Content-Type") == "application/x-www-form-urlencoded") {
    parser = new HTTPURLEncodedBodyParser(req);
  }
  if (parser != NULL) {
    while (parser->nextField()) {
      std::string name = parser->getFieldName();
      if (name == "mode") {
        mode = web_parameter(parser).toInt();
      }
      if (name == "target") {
        target = web_parameter(parser).toFloat();
      }
    }
  } else {
    mode = web_parameter(req, "mode").toInt();
    target = web_parameter(req, "target").toFloat();
  }
  if (web_request_post(req)) {
    log_i("Setting mode = %d", mode);
    log_i("Setting target = %f", target);
    if (target < THERMO_TARGET_MIN || target > THERMO_TARGET_MAX) {
      target = THERMO_TARGET_DEF;
    }
    if (mode != THERMO_MODE_AUTO) {
      thermo_auto_last = 0;
      thermo_auto_fail = 0;
    }
    thermo_boost_stop = 0;
    thermo_boost_begin = "";
    thermo_boost_end = "";
    int boost = 0;
    switch (mode) {
      case THERMO_MODE_BOOST30:
        boost = 30;
        break;
      case THERMO_MODE_BOOST60:
        boost = 60;
        break;
      case THERMO_MODE_BOOST90:
        boost = 90;
        break;
    }
    if (boost != 0) {
      if (thermo_mode <= THERMO_MODE_AUTO) {
        thermo_boost_reset = thermo_mode;
      }
      thermo_boost_stop = millis();
      log_i("Boost start at %d", thermo_boost_stop);
      thermo_boost_stop += boost * 60000;
      log_i("Boost will end at %d", thermo_boost_stop);
      struct tm timeinfo;
      if (wifi_time_read(&timeinfo)) {
        char time_str[100];
        strftime(time_str,sizeof(time_str),"%Y-%m-%dT%H:%M",&timeinfo);
        thermo_boost_begin = String(time_str);
        time_t epoch = mktime(&timeinfo) + boost * 60;
        localtime_r(&epoch, &timeinfo);
        strftime(time_str,sizeof(time_str),"%Y-%m-%dT%H:%M",&timeinfo);
        thermo_boost_end = String(time_str);
      }
    }
    thermo_mode = mode;
    thermo_target = target;
    preferences.putUChar(PREF_THERMO_MODE, thermo_mode);
    preferences.putFloat(PREF_THERMO_TARGET, thermo_target);
    log_i("Saved thermo_mode = %d", thermo_mode);
    log_i("Saved thermo_target = %f", thermo_target);
    return web_send_redirect(req,res,"/");
  }
  JSONVar data = html_data();
  String time_ref = data["time_ref"];
  String temp = data["temp"];
  String hum = data["hum"];
  String boiler_status = data["boiler_status"];
  String html = "<body>";
  html += "<div style=\"height:100vh;display:flex;flex-direction:column-reverse;justify-content:center;\"><div style=\"padding-bottom:3rem\" class=\"text-center\">";
  html += "<div><span id=\"time_ref\">"+time_ref+"<span></div>";
  html += "<div class=\"fs-03 fw-bold\"><span id=\"temp\">"+temp+"<span></div>";
  html += "<div class=\"fs-1\">Umidit&agrave;: <span id=\"hum\">"+hum+"<span></div>";
  html += "<div class=\"fs-1\">Riscaldamento: <span id=\"boiler_status\">"+boiler_status+"<span></div>";
  html += "</div></div>";
  html += "<hr />";
  String target_class = "d-none";
  String boost_class = "d-none";
  switch (thermo_mode) {
    case THERMO_MODE_AUTO:
      target_class = "";
      break;
    case THERMO_MODE_BOOST30:
    case THERMO_MODE_BOOST60:
    case THERMO_MODE_BOOST90:
      boost_class = "";
      break;
  }
  html += "<form method=\"post\" id=\"settings\" style=\"margin:auto;max-width:640\">";
  html += "<fieldset>";
  html += "<div class=\"input-group\">";
  html += "<span class=\"input-group-text\">Modalit&agrave;</span>";
  html += "<select name=\"mode\" class=\"form-select\">";
  html += "<option value=\""+String(THERMO_MODE_OFF)+"\">OFF</option>";
  html += "<option value=\""+String(THERMO_MODE_AUTO)+"\">Automatica</option>";
  html += "<option value=\""+String(THERMO_MODE_BOOST30)+"\">Boost 30'</option>";
  html += "<option value=\""+String(THERMO_MODE_BOOST60)+"\">Boost 60'</option>";
  html += "<option value=\""+String(THERMO_MODE_BOOST90)+"\">Boost 90'</option>";
  html += "</select></div>";
  html += "<div class=\"input-group "+target_class+"\"><span class=\"input-group-text\">Temperatura</span><input type=\"number\" name=\"target\" value=\""+String(thermo_target)+"\" min=\""+THERMO_TARGET_MIN+"\" max=\""+THERMO_TARGET_MAX+"\" step=\"0.1\" class=\"form-control\" /></div>";
  html += "<div class=\"input-group "+boost_class+"\"><span class=\"input-group-text\">Inizio Boost</span><input type=\"datetime-local\" name=\"boostBegin\" value=\""+thermo_boost_begin+"\" readonly=\"readonly\" class=\"form-control\" /></div>";
  html += "<div class=\"input-group "+boost_class+"\"><span class=\"input-group-text\">Fine Boost</span><input type=\"datetime-local\" name=\"boostEnd\" value=\""+thermo_boost_end+"\" readonly=\"readonly\" class=\"form-control\" /></div>";
  html += "<div class=\"text-center\"><button type=\"submit\" class=\"btn btn-primary\">Salva</button> <button type=\"button\" onclick=\"location.reload()\" class=\"btn btn-primary\">Annulla</button></div>";
  html += "</fieldset>";
  html += "</form>";
  html += "<form method=\"post\" action=\""+String(CONF_WEB_URI_LOGOUT)+"\" style=\"margin:auto;max-width:640\">";
  html += "<div class=\"text-center\" style=\"margin-bottom:1.5rem\"><button type=\"button\" onclick=\"location='"+String(CONF_WEB_URI_CONFIG)+"'\" class=\"btn btn-secondary\">Configurazione di sistema</button> <button type=\"submit\" class=\"btn btn-secondary\">Logout</button></div>";
  html += "</form>";
  html += "<script>";
  html += "document.addEventListener('DOMContentLoaded', function() {";
  html += "function toggleClass(el,c,b) {if(b) el.classList.add(c); else el.classList.remove(c);}";
  html += "var form = document.getElementById('settings');";
  html += "form.mode.value='"+String(thermo_mode)+"';";
  html += "form.mode.addEventListener('change', function(ev) {";
  html += "var temp = false;";
  html += "var boost = false;";
  html += "switch(ev.target.value) {";
  html += "case '"+String(THERMO_MODE_AUTO)+"':temp = true;break;";
  html += "case '"+String(THERMO_MODE_BOOST30)+"':";
  html += "case '"+String(THERMO_MODE_BOOST60)+"':";
  html += "case '"+String(THERMO_MODE_BOOST90)+"':";
  html += "boost = true;break;";
  html += "}";
  html += "toggleClass(form.target.parentElement,'d-none',!temp);";
  html += "toggleClass(form.boostBegin.parentElement,'d-none',!boost);";
  html += "toggleClass(form.boostEnd.parentElement,'d-none',!boost);";
  html += "});form.mode.dispatchEvent(new Event('change'));setInterval(function(){";
  html += "var xhr = new XMLHttpRequest();";
  html += "xhr.onreadystatechange = function() {if(xhr.readyState==4&&xhr.status==200) {";
  html += "var data = JSON.parse(xhr.responseText);";
  html += "document.getElementById('time_ref').innerHTML = data.time_ref;";
  html += "document.getElementById('temp').innerHTML = data.temp;";
  html += "document.getElementById('hum').innerHTML = data.hum;";
  html += "document.getElementById('boiler_status').innerHTML = data.boiler_status;";
  html += "}};xhr.open('GET','"+String(CONF_WEB_URI_REST_DATA)+"',true);xhr.send();";
  html += "},"+String(thermo_refresh)+");});";
  html += "</script></body>";
  web_send_page(req,res,html_title,html);
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
    for (int i = 0; i < 100; i++) {
      web_server_loop();
      delay(10);
    }
  } else {
    delay(1000);
  }
#endif
  uint32_t now = millis();
  //log_d("now = %d", now);
  switch (thermo_mode) {
    case THERMO_MODE_OFF:
      boiler_write(false);
      break;
    case THERMO_MODE_AUTO:
      if (!dht_available()) {
        thermo_mode = THERMO_MODE_OFF;
        break;
      }
      if (now >= thermo_auto_last + thermo_auto_interval) {
        thermo_auto_last = now;
        float temp = dht_read_temperature(true);
        if (isnan(temp)) {
          if (thermo_auto_fail == 0) {
            thermo_auto_fail = now;
          }
          if (now >= thermo_auto_fail + 30 * 60000) {
            thermo_mode = THERMO_MODE_OFF;
          }
          break;
        }
        thermo_auto_fail = 0;
        boiler_write(temp <= thermo_target);
      }
      break;
    case THERMO_MODE_BOOST30:
    case THERMO_MODE_BOOST60:
    case THERMO_MODE_BOOST90:
      //log_d("Boost stop = %d", thermo_boost_stop);
      if (thermo_boost_stop > 0 && now < thermo_boost_stop) {
        boiler_write(true);
      } else {
        log_i("Boost end");
        thermo_mode = thermo_boost_reset;
      }
      break;
  }
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
  boiler_pin = preferences.getUChar(PREF_BOILER_PIN);
  if (boiler_pin != 0) {
    pinInit(boiler_pin, OUTPUT);
    digitalWriteState(boiler_pin, LOW);
  }
  thermo_mode = preferences.getUChar(PREF_THERMO_MODE,THERMO_MODE_OFF);
  if (thermo_mode > THERMO_MODE_AUTO) {
    thermo_mode = THERMO_MODE_AUTO;
  }
  thermo_target = preferences.getFloat(PREF_THERMO_TARGET, THERMO_TARGET_DEF);
  thermo_auto_interval = preferences.getULong(PREF_THERMO_AUTO_INTERVAL,CONF_THERMO_AUTO_INTERVAL);
  thermo_refresh = preferences.getULong(PREF_THERMO_REFRESH);
  if (thermo_refresh < CONF_THERMO_REFRESH_MIN) {
    thermo_refresh = CONF_THERMO_REFRESH_MIN;
  }
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
  bool web_secure = false;
#ifdef CONF_WEB_HTTPS
  web_server_setup_https(preferences.getString(PREF_WEB_CERT),preferences.getString(PREF_WEB_CERT_KEY));
  web_secure = preferences.getBool(PREF_WEB_SECURE);
#endif
  web_users_setup(
    preferences.getString(PREF_WEB_ADMIN_USERNAME,CONF_WEB_ADMIN_USERNAME).c_str(),
    preferences.getString(PREF_WEB_ADMIN_PASSWORD,CONF_WEB_ADMIN_PASSWORD).c_str()
  );
  web_ota_setup();
  web_config_setup(preferences.getBool(PREF_CONFIG_PUBLISH));
  web_server_register(HTTP_ANY, CONF_WEB_URI_REST_DATA, &web_handle_rest_read);
  web_server_register(HTTP_ANY, "/", &web_handle_root);
  web_server_begin(preferences.getString(PREF_WIFI_NAME,CONF_WIFI_NAME).c_str(), web_secure);
#endif
}

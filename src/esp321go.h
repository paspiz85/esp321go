
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
#include "openhab.h"
#endif
#ifdef CONF_WEB
#include "web_gui.h"
#include "web_config.h"
#ifdef CONF_ADMIN_WEB_OTA
#include "web_ota.h"
#endif
#ifdef CONF_WEB_USERS
#include "web_users.h"
#endif
#endif
#ifdef CONF_ADMIN_ARDUINO_OTA
#include <ArduinoOTA.h>
#endif
#ifdef CONF_NEOPIXEL
#include <Adafruit_NeoPixel.h>
#endif

#define THERMO_MODE_OFF     (0)
#define THERMO_MODE_AUTO    (1)
#define THERMO_MODE_BOOST30 (3)
#define THERMO_MODE_BOOST60 (4)
#define THERMO_MODE_BOOST90 (5)
#define THERMO_TARGET_MIN   (7)
#define THERMO_TARGET_MAX   (30)
#define THERMO_TARGET_DEF   (15)

/**
 * @see https://github.com/adafruit/Adafruit_NeoPixel/blob/master/Adafruit_NeoPixel.h
 */

uint8_t log_level;
uint32_t reboot_free;
uint32_t reboot_ms;

String admin_username;
String admin_password;

#ifdef CONF_NEOPIXEL
Adafruit_NeoPixel * pixels = NULL;
#endif

uint8_t boiler_pin = 0;

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

#ifdef CONF_WIFI
String wifi_name;
uint8_t wifi_ap_pin = 0;
OpenHAB* openhab = NULL;
String openhab_bus_item;
#endif

String digitalString(int value) {
  return value == HIGH ? "HIGH" : "LOW";
}
float read_temperature(bool force = false, float* bmp_temp = NULL, float* dht_temp = NULL) {
  float temp;
  float sum = 0;
  int count = 0;
#ifdef CONF_BMP280
  temp = bmp280_read_temperature();
  if (!isnan(temp)) {
    sum += temp;
    count++;
    if (bmp_temp != NULL) {
      *bmp_temp = temp;
    }
  }
#endif
#ifdef CONF_DHT
  temp = dht_read_temperature(force || count > 0);
  if (!isnan(temp)) {
    sum += temp;
    count++;
    if (dht_temp != NULL) {
      *dht_temp = temp;
    }
  }
#endif
  if (count == 0) {
    temp = NAN;
  } else {
    temp = sum / count;
  }
  return temp;
}

void boiler_write(bool b) {
  if (boiler_pin != 0) {
    if (b) {
      if (PinMemory.getPinState(boiler_pin) == LOW) {
        PinMemory.writeDigital(boiler_pin, HIGH);
      }
    } else {
      if (PinMemory.getPinState(boiler_pin) != LOW) {
        PinMemory.writeDigital(boiler_pin, LOW);
      }
    }
  }
}

void items_publish(JSONVar data) {
#ifdef CONF_WIFI
  if (!WiFiUtils::isConnected()) {
    return;
  }
  if (openhab != NULL) {
    JSONVar source;
#ifdef PLATFORM_ESP32
    source["type"] = "esp32";
#endif
#ifdef PLATFORM_ESP8266
    source["type"] = "esp8266";
#endif
    if (wifi_name != "") {
      source["name"] = wifi_name;
    }
    source["wifi"] = WiFi.SSID();
    if (log_level >= ESP_LOG_DEBUG) {
      source["signal"] = WiFi.RSSI();
    }
    source["ipAddress"] = WiFiUtils::getIP();
    JSONVar message;
    message["source"] = source;
    message["data"] = data;
    openhab->item_write(openhab_bus_item,JSON.stringify(message));
  }
  // TODO gestire MQTT ?
#endif
}

#ifdef CONF_WEB
WebGUI* web_gui;

bool web_admin_authenticate(HTTPRequest* req) {
  return web_authenticate(req, admin_username.c_str(), admin_password.c_str());
}

String web_html_footer(bool admin) {
  String html = "<hr/><div>";
  html += html_encode(WiFiUtils::getInfo());
  html += " - ";
  html += "Memory Free: "+String(ESP.getFreeHeap());
  html += " - Uptime: "+String(millis())+"</div>";
  html += "<div style=\"margin-top:1rem\">";
  html += "<a href=\""+String(PROJECT_URL)+"\">"+String(PROJECT_URL)+"</a>";
  html += "</div>";
  html += "<div style=\"margin-top:1rem\">";
  html += String(COMPILE_VERSION)+" ["+String(__TIMESTAMP__)+"]";
#ifdef CONF_ADMIN_WEB_OTA
  if (admin) {
    html += " <button class=\"btn btn-secondary\" style=\"font-size:0.75rem;padding:0.25rem 0.5rem\" onclick=\"location='"+WebOTA::getUri()+"'\">Update</button>";
  }
#endif
  html += "</div>";
  return html;
}

JSONVar html_data() {
  JSONVar data;
  struct tm timeinfo;
  if (WiFiTime::read(&timeinfo)){
    char time_str[100];
    strftime(time_str,sizeof(time_str),"%H:%M:%S - %A %e %B %Y",&timeinfo);
    data["time_ref"] = String(time_str);
  } else {
    data["time_ref"] = "Temperatura attuale";
  }
  float bmp_temp = NAN;
  float dht_temp = NAN;
  float temp = read_temperature(false,&bmp_temp,&dht_temp);
  if (isnan(temp)) {
    data["temp"] = "--";
  } else {
    data["temp"] = String(temp,1) + " &deg;C";
  }
  if (!isnan(bmp_temp)) {
    data["bmp_temp"] = String(bmp_temp,1) + " &deg;C";
  }
  if (!isnan(dht_temp)) {
    data["dht_temp"] = String(dht_temp,1) + " &deg;C";
  }
#ifdef CONF_DHT
  temp = dht_read_humidity();
  if (isnan(temp)) {
    data["hum"] = "--";
  } else {
    data["hum"] = String(temp,0) + "%";
  }
#else
  data["hum"] = "--";
#endif
  if (boiler_pin == 0) {
    data["boiler_status"] = "--";
  } else {
    data["boiler_status"] = PinMemory.getPinState(boiler_pin) == LOW ? "OFF" : "ON";
  }
  return data;
}

void web_handle_rest_read(HTTPRequest * req, HTTPResponse * res) {
  return web_sendResponse(req,res,200,"application/json",JSON.stringify(html_data()));
}

void web_handle_root(HTTPRequest * req, HTTPResponse * res) {
  if (!WebUsers::login(req,res)) {
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
        mode = web_getParameter(parser).toInt();
      }
      if (name == "target") {
        target = web_getParameter(parser).toFloat();
      }
    }
  } else {
    mode = web_getParameter(req, "mode").toInt();
    target = web_getParameter(req, "target").toFloat();
  }
  if (web_isRequestMethodPost(req)) {
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
      if (WiFiTime::read(&timeinfo)) {
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
    return web_sendRedirect(req,res,"/");
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
  web_gui->sendPage(req,res,web_gui->getTitle(),html);
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
  WiFiUtils::loopToHandleConnection(CONF_WIFI_MODE_LIMIT);
  WiFiTime::loopToSynchronize();
#ifdef CONF_ADMIN_ARDUINO_OTA
  ArduinoOTA.handle();
#endif
#endif
#ifdef CONF_WEB
  web_gui->loopToHandleClients();
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
        float temp = read_temperature(true);
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
  preferences.begin("my-app", false);
  //preferences.putUChar(PREF_LOG_LEVEL,ESP_LOG_DEBUG);
  //preferences.putString("wifi2_ssid","");
  //preferences.putString("wifi2_pswd","");
  log_level = preferences.getUChar(PREF_LOG_LEVEL,CONF_LOG_LEVEL);
  if (log_level > ESP_LOG_DEBUG) {
    log_level = ESP_LOG_DEBUG;
  }
  Serial.println("Log level : " + log_level);
  esp_log_level_set("*",(esp_log_level_t)log_level);
  reboot_free = preferences.getULong(PREF_REBOOT_FREE);
  reboot_ms = preferences.getULong(PREF_REBOOT_MS);
  admin_username = preferences.getString(PREF_ADMIN_USERNAME,CONF_ADMIN_USERNAME);
  admin_password = preferences.getString(PREF_ADMIN_PASSWORD,CONF_ADMIN_PASSWORD);
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
#ifdef CONF_NEOPIXEL
  uint16_t pixels_num = preferences.getUShort(PREF_NEOPIXEL_NUM);
  uint8_t pixels_pin = preferences.getUChar(PREF_NEOPIXEL_PIN);
  if (pixels_num > 0) {
    pixels = new Adafruit_NeoPixel(pixels_num,pixels_pin,CONF_NEOPIXEL_TYPE);
    pixels->begin();
    for(int i=0;i<pixels->numPixels();i++){
      pixels->setPixelColor(i,pixels->Color(0,0,0));
    }
    pixels->show();
  }
#endif
  boiler_pin = preferences.getUChar(PREF_BOILER_PIN);
  if (boiler_pin != 0) {
    pinMode(boiler_pin, OUTPUT);
    PinMemory.writeDigital(boiler_pin, LOW);
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
  wifi_name = preferences.getString(PREF_WIFI_NAME,CONF_WIFI_NAME);
  wifi_ap_pin = preferences.getUChar(PREF_WIFI_AP_PIN);
  if (wifi_ap_pin != 0) {
    pinMode(wifi_ap_pin, OUTPUT);
  }
  for (uint8_t i = 1; i <= CONF_WIFI_COUNT; i++) {
    String ssid = preferences.getString((PREF_PREFIX_WIFI+String(i)+PREF_PREFIX_WIFI_SSID).c_str());
    String pswd = preferences.getString((PREF_PREFIX_WIFI+String(i)+PREF_PREFIX_WIFI_PSWD).c_str());
    if (ssid != "") {
      WiFiUtils::addAP(ssid.c_str(),pswd.c_str());
    }
  }
  WiFiUtils::setup(
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
    });
  delay(1000);
  WiFiTime::setup(CONF_WIFI_NTP_SERVER,
    CONF_WIFI_NTP_INTERVAL,
    preferences.getString(PREF_TIME_ZONE,CONF_TIME_ZONE).c_str());
#ifdef CONF_ADMIN_ARDUINO_OTA
  ArduinoOTA.setPassword(admin_password.c_str());
  ArduinoOTA.begin();
#endif
  String openhab_rest_uri = preferences.getString(PREF_OPENHAB_REST_URI);
  if (openhab_rest_uri != "") {
    openhab = new OpenHAB(openhab_rest_uri);
  }
  openhab_bus_item = preferences.getString(PREF_OPENHAB_BUS_ITEM);
  if (openhab_bus_item == "") {
    openhab_bus_item = CONF_OPENHAB_BUS_ITEM;
  }
#endif
#ifdef CONF_WEB
  String web_html_title = preferences.getString(PREF_WEB_HTML_TITLE);
  if (web_html_title == "") {
    web_html_title = CONF_WEB_HTML_TITLE;
  }
  bool web_secure = false;
#ifdef CONF_WEB_HTTPS
  web_gui = new WebGUI((uint16_t)80,(uint16_t)443,preferences.getString(PREF_WEB_CERT),preferences.getString(PREF_WEB_CERT_KEY));
#else
  web_gui = new WebGUI();
  web_secure = preferences.getBool(PREF_WEB_SECURE);
#endif
  web_gui->setTitle(web_html_title);
  web_gui->setFooter(web_html_footer);
  WebReset* web_reset = new WebReset(web_gui,CONF_WEB_URI_RESET);
  WebConfig::setup(web_gui,web_reset,CONF_WEB_URI_CONFIG,preferences.getBool(PREF_CONFIG_PUBLISH));
#ifdef CONF_ADMIN_WEB_OTA
  WebOTA::setup(web_gui,CONF_WEB_URI_FIRMWARE_UPDATE);
#endif
#ifdef CONF_WEB_USERS
  WebUsers::setup(web_gui,admin_username,admin_password,preferences.getString(PREF_WEB_USERS));
#endif
  web_gui->handle(HTTP_ANY, CONF_WEB_URI_REST_DATA, &web_handle_rest_read);
  web_gui->handle(HTTP_ANY, "/", &web_handle_root);
  web_gui->begin(wifi_name.c_str(),web_secure);
#endif
}

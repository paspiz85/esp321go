
#include "base_conf.h"
#ifdef CONF_WIFI
#include "wifi.h"
#include "wifi_time.h"
#endif
#ifdef CONF_WEB
#include "web.h"
#include "web_templates.h"
#endif

#ifdef CONF_WIFI
void wifi_ap_state_changed(int value, bool skip_publish) {
#ifdef CONF_WIFI_AP_PIN
  // TODO 
  //digitalWriteState(CONF_WIFI_AP_PIN, value, skip_publish);
  digitalWrite(CONF_WIFI_AP_PIN, value);
#endif
}
#endif

#ifdef CONF_WEB
String web_html_title() {
  return CONF_WEB_HTML_TITLE;
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
  String html = "<body style=\"text-align:center\"><h1>"+web_html_title()+"</h1>";
  html += "Hello";
  html += "</body>";
  web_send_page(web_html_title(),html,refresh);
}
#endif

void loop() {
  Serial.println("UP");
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);         
  Serial.println("DOWN");              // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}



void setup() {
  Serial.begin(CONF_MONITOR_BAUD_RATE);
  while (! Serial);


// TODO setup logs
  pinMode(LED_BUILTIN, OUTPUT);



#ifdef CONF_WIFI
#ifdef CONF_WIFI_AP_PIN
  pinMode(CONF_WIFI_AP_PIN, OUTPUT);
#endif
  JSONVar wiki_networks = JSON.parse(CONF_WIFI_NETWORKS);
  JSONVar wiki_networks_keys = wiki_networks.keys();
  for (int i = 0; i < wiki_networks_keys.length(); i++) {
    wifi_add_ap(wiki_networks_keys[i],wiki_networks[wiki_networks_keys[i]]);
  }
  wifi_setup(
    CONF_WIFI_MODE,
    CONF_WIFI_AP_IP,
    CONF_WIFI_AP_SSID,
    CONF_WIFI_AP_PSWD,
    CONF_WIFI_CONN_TIMEOUT_MS,
    CONF_WIFI_CHECK_INTERVAL_MIN,
    CONF_WIFI_CHECK_THRESHOLD
  );
  delay(1000);
  wifi_time_setup(CONF_WIFI_NTP_SERVER, CONF_WIFI_NTP_INTERVAL, CONF_TIME_ZONE);
#endif
#ifdef CONF_WEB
  web_server_setup_http();
  //web_admin_setup(CONF_WEB_ADMIN_USERNAME,CONF_WEB_ADMIN_PASSWORD);
  web_server_register(HTTP_ANY, "/", web_handle_root);
  web_server_begin(CONF_WIFI_NAME);
#endif
}
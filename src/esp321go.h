
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
#include "openhab.h"
#endif
#include "types.h"

uint32_t reboot_free;
uint32_t reboot_ms;

int pin_states[CONF_SCHEMA_PIN_COUNT];
int8_t pin_channel[CONF_SCHEMA_PIN_COUNT];
uint32_t pin_write_last_ms[CONF_SCHEMA_PIN_COUNT];
Input inputs[CONF_SCHEMA_INPUT_COUNT];
Output outputs[CONF_SCHEMA_OUTPUT_COUNT];
const Output * pin_output[CONF_SCHEMA_PIN_COUNT] = {NULL};
String wifi_name;
uint8_t wifi_ap_pin = 0;
bool dht_publish;

String html_title;

String openhab_rest_uri;
String openhab_bus_item;
uint32_t input_read_interval_ms;
uint32_t input_read_last_ms = 0;
uint32_t publish_interval_ms;
uint32_t publish_last_ms = 0;
bool publish_ip;
bool publish_ssid;
bool publish_rssi;

void items_publish(JSONVar data) {
#ifdef CONF_WIFI
  if (!wifi_have_internet()) {
    return;
  }
  if (openhab_rest_uri != "") {
    JSONVar source;
    source["type"] = "esp32";
    if (wifi_name != "") {
      source["name"] = wifi_name;
    }
    if (publish_ssid) {
      source["wifi"] = WiFi.SSID();
    }
    if (publish_rssi) {
      source["signal"] = WiFi.RSSI();
    }
    if (publish_ip) {
      source["ipAddress"] = wifi_get_ip_address();
    }
    JSONVar message;
    message["source"] = source;
    message["data"] = data;
    openhab_item_state_write(openhab_rest_uri,openhab_bus_item,JSON.stringify(message));
  }
  // TODO gestire MQTT ?
#endif
}

void on_pin_read(uint8_t pin,int value,input_type_t type,bool change=false);
void on_pin_read(uint8_t pin,double value,input_type_t type,bool change=false);
void on_output_write(uint8_t num,bool reset=false);

void pinInit(int pin,int mode) {
  pinMode(pin,mode);
  pin_states[pin] = -1;
  if (mode == OUTPUT) {
    pin_write_last_ms[pin] = 0;
  }
  pin_output[pin] = NULL;
}

String digitalString(int value) {
  return value == HIGH ? "HIGH" : "LOW";
}

void digitalWriteState(uint8_t pin, int value, bool skip_publish = false) {
  digitalWrite(pin,value);
  pin_states[pin] = value;
  pin_write_last_ms[pin] = millis();
  const Output * output = pin_output[pin];
  if (output != NULL) {
    if (output->stored) {
      preferences.putInt((output->key).c_str(),value);
    }
    if (!skip_publish && output->published) {
      item_publish((output->name).c_str(),digitalString(value));
    }
  }
}

void analogWriteState(uint8_t pin, uint16_t value, bool skip_publish = false) {
  ledcWrite(pin_channel[pin], value);
  pin_states[pin] = value;
  pin_write_last_ms[pin] = millis();
  const Output * output = pin_output[pin];
  if (output != NULL) {
    if (output->stored) {
      preferences.putInt((output->key).c_str(),value);
    }
    if (!skip_publish && output->published) {
      item_publish((output->name).c_str(),value);
    }
  }
}

void toneState(uint8_t pin,uint32_t value,bool skip_publish=false) {
  ledcWriteTone(pin_channel[pin], value);
  pin_states[pin] = value;
  pin_write_last_ms[pin] = millis();
  const Output * output = pin_output[pin];
  if (output != NULL) {
    if (output->stored) {
      preferences.putInt((output->key).c_str(),value);
    }
    if (!skip_publish && output->published) {
      item_publish((output->name).c_str(),value);
    }
  }
}

int output_var_type(output_type_t type) {
  switch(type) {
    case UINT8_VAR:  return UINT8;
    case UINT16_VAR: return UINT16;
    case UINT32_VAR: return UINT32;
    case UINT64_VAR: return UINT64;
    case INT8_VAR:   return INT8;
    case INT16_VAR:  return INT16;
    case INT32_VAR:  return INT32;
    case INT64_VAR:  return INT64;
    case STRING_VAR: return STRING;
    case BOOL_VAR:   return BOOL;
    case FLOAT_VAR:  return FLOAT;
    case DOUBLE_VAR: return DOUBLE;
    default: return 0;
  }
}

String output_write(const Output * output,String input_value,bool reset=false) {
  int8_t pin = output->pin;
  if (output->type == DIGITAL_OUTPUT && pin >= 0) {
    if (input_value == "") {
      if (pin_states[pin] == HIGH) {
        digitalWriteState(pin,LOW);
      } else {
        digitalWriteState(pin,HIGH);
      }
      on_output_write(output->num);
    } else if (input_value == "HIGH") {
      digitalWriteState(pin,HIGH);
      on_output_write(output->num);
    } else if (input_value == "LOW") {
      digitalWriteState(pin,LOW);
      on_output_write(output->num);
    }
  } else if (output->type == PULSE_1_OUTPUT && pin >= 0) {
    digitalWriteState(pin,HIGH);
    on_output_write(output->num);
  } else if (output->type == ANALOG_PWM_OUTPUT && pin >= 0) {
    if (input_value == "") {
      return "Missing value";
    }
    for (uint8_t j = 0; j < input_value.length(); j++) {
      if (!isDigit(input_value.charAt(j))) {
        return "Invalid value: " + input_value;
      }
    }
    int value = input_value.toInt();
    if (value < 0 || value >= (1<<CONF_SCHEMA_PWM_RES)) {
      return "Out-of-bound value: " + input_value;
    }
    analogWriteState(pin,value);
    on_output_write(output->num);
  } else if (output->type == ANALOG_FM_OUTPUT && pin >= 0) {
    int value = 0;
    if (!reset) {
      if (input_value == "") {
        return "Missing value";
      }
      for (uint8_t j = 0; j < input_value.length(); j++) {
        if (!isDigit(input_value.charAt(j))) {
          return "Invalid value: " + input_value;
        }
      }
      value = input_value.toInt();
      if (value < 0) {
        return "Out-of-bound value: " + input_value;
      }
    }
    toneState(pin,value);
    on_output_write(output->num);
  } else {
    int type = output_var_type(output->type);
    if (type != 0) {
      String publish_key = "";
      if (output->published) {
        if (output->name == "") {
          publish_key = output->key;
        } else {
          publish_key = output->name;
        }
      }
      if (reset) {
        preferences_remove((output->key).c_str(),(ctype_t)type,publish_key);
        on_output_write(output->num,true);
      } else {
        if (type == BOOL && input_value == "") {
          input_value = preferences.getBool((output->key).c_str()) ? "false" : "true"; 
        }
        preferences_put((output->key).c_str(),(ctype_t)type,input_value,publish_key);
        on_output_write(output->num);
      }
    }
  }
  return "";
}

float mq2ResistanceCalculation(int raw_adc, float rl) {
  if (raw_adc == 0) {
    raw_adc = 1;
  }
  return rl*(CONF_ANALOG_READ_MAX-raw_adc)/raw_adc;
}

float mq2Read(uint8_t pin, bool skipRules = false, float rl = CONF_MQ2_RL_VALUE) {
  uint8_t n = CONF_MQ2_READ_SAMPLE_TIMES;
  uint8_t i;
  float rs=0;
  for (i=0;i<n;i++) {
    rs += mq2ResistanceCalculation(analogRead(pin),rl);
    delay(CONF_MQ2_READ_SAMPLE_INTERVAL);
  }
  rs = rs/n;
  if (!skipRules) {
    on_pin_read(pin,(double)rs,MQ2);
  }
  return rs;
}

int analogReadRuled(uint8_t pin) {
  int rs = analogRead(pin);
  on_pin_read(pin,rs,ANALOG_INPUT);
  return rs;
}

int digitalReadRuled(uint8_t pin) {
  int rs = digitalRead(pin);
  on_pin_read(pin,rs,DIGITAL_INPUT);
  return rs;
}

String input_get(const Input * input, bool fromRules = false) {
  String str = "";
  switch (input->type) {
    case DIGITAL_INPUT:
      if (input->pin >= 0) {
        if (fromRules) {
          str = String(digitalRead(input->pin));
        } else {
          str = digitalString(digitalReadRuled(input->pin));
        }
      }
      break;
    case ANALOG_INPUT:
      if (input->pin >= 0) {
        if (fromRules) {
          str = String(analogRead(input->pin));
        } else {
          str = String(analogReadRuled(input->pin));
        }
      }
      break;
    case MQ2:
      if (input->pin >= 0) {
        str = String(mq2Read(input->pin,fromRules));
      }
      break;
  }
  return str;
}

String output_get(const Output * output) {
  String str = "";
  const char * key = (output->key).c_str();
  if (output->type == DIGITAL_OUTPUT || output->type == PULSE_1_OUTPUT || output->type == ANALOG_PWM_OUTPUT || output->type == ANALOG_FM_OUTPUT) {
    int value;
    if (output->pin >= 0) {
      value = pin_states[output->pin];
    } else {
      value = preferences.getInt(key);
    }
    if (output->type == DIGITAL_OUTPUT || output->type == PULSE_1_OUTPUT) {
      str = digitalString(value);
    } else {
      str = String(value);
    }
  } else {
    int type = output_var_type(output->type);
    if (type != 0 && preferences.isKey(key)) {
      str = preferences_get(key,(ctype_t)type);
    }
  }
  return str;
}

void publish() {
  JSONVar message;
  for (uint8_t i = 0; i < CONF_SCHEMA_INPUT_COUNT; i++) {
    if (inputs[i].type == NO_INPUT || !(inputs[i].published)) {
      continue;
    }
    switch (inputs[i].type) {
      case DIGITAL_INPUT:
        if (inputs[i].pin >= 0) {
          message[inputs[i].name] = digitalString(digitalReadRuled(inputs[i].pin));
        }
        break;
      case ANALOG_INPUT:
        if (inputs[i].pin >= 0) {
          message[inputs[i].name] = analogReadRuled(inputs[i].pin);
        }
        break;
      case MQ2:
        if (inputs[i].pin >= 0) {
          message[inputs[i].name] = mq2Read(inputs[i].pin);
        }
        break;
    }
  }
  for (uint8_t i = 0; i < CONF_SCHEMA_OUTPUT_COUNT; i++) {
    if (outputs[i].type == NONE || !(outputs[i].published)) {
      continue;
    }
    const char * key = outputs[i].key.c_str();
    if (outputs[i].type == DIGITAL_OUTPUT || outputs[i].type == PULSE_1_OUTPUT || outputs[i].type == ANALOG_PWM_OUTPUT || outputs[i].type == ANALOG_FM_OUTPUT) {
      int value;
      if (outputs[i].pin >= 0) {
        value = pin_states[outputs[i].pin];
      } else {
        value = preferences.getInt(key);
      }
      if (outputs[i].type == DIGITAL_OUTPUT || outputs[i].type == PULSE_1_OUTPUT) {
        message[outputs[i].name] = digitalString(value);
      } else {
        message[outputs[i].name] = value;
      }
    } else {
      if (preferences.isKey(key)) {
        switch (outputs[i].type) {
          case UINT8_VAR:
            message[outputs[i].name] = preferences.getUChar(key);
            break;
          case UINT16_VAR:
            message[outputs[i].name] = preferences.getUShort(key);
            break;
          case UINT32_VAR:
            message[outputs[i].name] = preferences.getULong(key);
            break;
          case UINT64_VAR:
            message[outputs[i].name] = uint64_to_string(preferences.getULong64(key));
            break;
          case INT8_VAR:
            message[outputs[i].name] = preferences.getChar(key);
            break;
          case INT16_VAR:
            message[outputs[i].name] = preferences.getShort(key);
            break;
          case INT32_VAR:
            message[outputs[i].name] = preferences.getLong(key);
            break;
          case INT64_VAR:
            message[outputs[i].name] = int64_to_string(preferences.getLong64(key));
            break;
          case STRING_VAR:
            message[outputs[i].name] = preferences.getString(key);
            break;
          case BOOL_VAR:
            message[outputs[i].name] = preferences.getBool(key);
            break;
          case FLOAT_VAR:
            message[outputs[i].name] = preferences.getFloat(key);
            break;
          case DOUBLE_VAR:
            message[outputs[i].name] = preferences.getDouble(key);
            break;
        }
      }
    }
  }
  if (dht_publish) {
    float dht_temp = dht_read_temperature();
    if (!isnan(dht_temp)) {
      message["temp"] = dht_temp;
    }
    float dht_hum = dht_read_humidity();
    if (!isnan(dht_hum)) {
      message["hum"] = dht_hum;
    }
  }
  int message_len = message.keys().length();
  log_d("message length is %d",message_len);
  if (message_len > 0) {
    items_publish(message);
  }
  publish_last_ms = millis();
}

#include "rules.h"

#ifdef CONF_WIFI
void wifi_ap_state_changed(int value, bool skip_publish) {
  if (wifi_ap_pin != 0 && pin_states[wifi_ap_pin] != value) {
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

void web_handle_root() {
  int refresh = web_parameter("refresh").toInt();
  String html = "<body style=\"text-align:center\"><h1>"+html_title+"</h1>";
  uint8_t input_count = 0;
  for (uint8_t i = 0; i < CONF_SCHEMA_INPUT_COUNT; i++) {
    if (inputs[i].type == NO_INPUT) {
      continue;
    }
    if (input_count++ == 0) {
      html += "<h2>Inputs</h2>";
      html += "<table style=\"margin:auto\">";
      html += "<tr><th>#</th><th>type</th><th>name</th><th>pin</th><th>value</th><th>m</th><th>p</th></tr>";
    }
    String input_type_str = "";
    String input_value = input_get(&inputs[i]);
    switch (inputs[i].type) {
      case DIGITAL_INPUT: input_type_str = "digital"; break;
      case ANALOG_INPUT: input_type_str = "analog"; break;
      case MQ2: input_type_str = "mq2"; break;
    }
    html += "<tr><td>"+String(inputs[i].num)+"</td><td>"+html_encode(input_type_str)+"</td><td>"+html_encode(inputs[i].name)+"</td><td>";
    html += (inputs[i].pin >= 0) ? String(inputs[i].pin) : "";
    html += "</td><td>"+input_value+"</td><td>";
    html += inputs[i].monitored ? "x" : "";
    html += "</td><td>";
    html += inputs[i].published ? "x" : "";
    html += "</td></tr>";
  }
  if (input_count > 0) {
    html += "</table>";
  }
  uint8_t out_count = 0;
  for (uint8_t i = 0; i < CONF_SCHEMA_OUTPUT_COUNT; i++) {
    if (outputs[i].type == NONE) {
      continue;
    }
    if (out_count++ == 0) {
      html += "<h2>Outputs</h2>";
      html += "<table style=\"margin:auto\">";
      html += "<tr><th>#</th><th>type</th><th>name</th><th>pin</th><th>value</th><th>s</th><th>p</th><th></th></tr>";
    }
    String output_type_str;
    String output_value_str = output_get(&outputs[i]);
    String output_editor = "";
    if (outputs[i].type == DIGITAL_OUTPUT) {
      output_type_str = "digital";
      output_editor = "<form action=\"/rest/out"+String(outputs[i].num)+"\" method=\"POST\" style=\"margin:0\">";
      output_editor += "<button type=\"submit\" class=\"btn btn-primary\">Toggle</button>";
      output_editor += "</form>";
    } else if (outputs[i].type == PULSE_1_OUTPUT) {
      output_type_str = "pulse";
      output_editor = "<form action=\"/rest/out"+String(outputs[i].num)+"\" method=\"POST\" style=\"margin:0\">";
      output_editor += "<button type=\"submit\" class=\"btn btn-primary\">Send</button>";
      output_editor += "</form>";
    } else if (outputs[i].type == ANALOG_PWM_OUTPUT) {
      output_type_str = "analog PWM";
      output_editor = "<form action=\"/rest/out"+String(outputs[i].num)+"\" method=\"POST\" style=\"margin:0\">";
      output_editor += "<input type=\"hidden\" name=\"reset\" value=\"false\" />";
      output_editor += "<button type=\"button\" class=\"btn btn-secondary toggleable\" onclick=\"Array.prototype.slice.call(this.form.getElementsByClassName('toggleable')).forEach(function(e){e.classList.toggle('d-none')})\">Edit</button>";
      output_editor += "<input type=\"number\" class=\"form-control toggleable d-none\" name=\"value\" value=\""+html_encode(output_value_str)+"\" min=\"0\" max=\""+String((1<<CONF_SCHEMA_PWM_RES)-1)+"\" />";
      output_editor += " <button type=\"submit\" class=\"btn btn-primary toggleable d-none\">Save</button>";
      output_editor += " <button type=\"button\" class=\"btn btn-secondary toggleable d-none\" onclick=\"Array.prototype.slice.call(this.form.getElementsByClassName('toggleable')).forEach(function(e){e.classList.toggle('d-none')})\">Cancel</button>";
      output_editor += "</form>";
    } else if (outputs[i].type == ANALOG_FM_OUTPUT) {
      output_type_str = "analog FM";
      output_editor = "<form action=\"/rest/out"+String(outputs[i].num)+"\" method=\"POST\" style=\"margin:0\">";
      output_editor += "<input type=\"hidden\" name=\"reset\" value=\"false\" />";
      output_editor += "<button type=\"button\" class=\"btn btn-secondary toggleable\" onclick=\"Array.prototype.slice.call(this.form.getElementsByClassName('toggleable')).forEach(function(e){e.classList.toggle('d-none')})\">Edit</button>";
      output_editor += "<input type=\"number\" class=\"form-control toggleable d-none\" name=\"value\" value=\""+html_encode(output_value_str)+"\" min=\"0\" />";
      output_editor += " <button type=\"submit\" class=\"btn btn-primary toggleable d-none\">Save</button>";
      output_editor += " <button type=\"submit\" class=\"btn btn-secondary toggleable d-none\" onclick=\"this.form.reset.value = 'true'\">Reset</button>";
      output_editor += " <button type=\"button\" class=\"btn btn-secondary toggleable d-none\" onclick=\"Array.prototype.slice.call(this.form.getElementsByClassName('toggleable')).forEach(function(e){e.classList.toggle('d-none')})\">Cancel</button>";
      output_editor += "</form>";
    } else {
      int type = output_var_type(outputs[i].type);
      if (type != 0) {
        output_type_str = ctype_str((ctype_t)type);
        if (type == BOOL) {
          output_editor = "<form action=\"/rest/out"+String(outputs[i].num)+"\" method=\"POST\" style=\"margin:0\">";
          output_editor += "<button type=\"submit\" class=\"btn btn-primary\">Toggle</button>";
          output_editor += "</form>";
        } else {
          output_editor = "<form action=\"/rest/out"+String(outputs[i].num)+"\" method=\"POST\" style=\"margin:0\">";
          output_editor += "<input type=\"hidden\" name=\"reset\" value=\"false\" />";
          output_editor += "<button type=\"button\" class=\"btn btn-secondary toggleable\" onclick=\"Array.prototype.slice.call(this.form.getElementsByClassName('toggleable')).forEach(function(e){e.classList.toggle('d-none')})\">Edit</button>";
          output_editor += html_input((ctype_t)type,"value",output_value_str,"class=\"toggleable d-none\"");
          output_editor += " <button type=\"submit\" class=\"btn btn-primary toggleable d-none\">Save</button>";
          output_editor += " <button type=\"submit\" class=\"btn btn-secondary toggleable d-none\" onclick=\"this.form.reset.value = 'true'\">Reset</button>";
          output_editor += " <button type=\"button\" class=\"btn btn-secondary toggleable d-none\" onclick=\"Array.prototype.slice.call(this.form.getElementsByClassName('toggleable')).forEach(function(e){e.classList.toggle('d-none')})\">Cancel</button>";
          output_editor += "</form>";
        }
      }
    }    
    html += "<tr><td>"+String(outputs[i].num)+"</td><td>"+output_type_str+"</td><td>"+html_encode(outputs[i].name)+"</td><td>";
    html += (outputs[i].pin >= 0) ? String(outputs[i].pin) : "";
    html += "</td><td>" + html_encode(output_value_str) + "</td><td>";
    html += outputs[i].stored ? "x" : "";
    html += "</td><td>";
    html += outputs[i].published ? "x" : "";
    html += "</td><td>" + output_editor + "</td></tr>";
  }
  if (out_count > 0) {
    html += "</table>";
  }
  if (dht_available()) {
    html += "<h2>DHT</h2>";
    html += "<table style=\"margin:auto\"><tr><th>type</th><th>pin</th><th>temp</th><th>hum</th><th>p</th></tr>";
    html += "<tr><td>"+String(preferences.getUChar(PREF_DHT_TYPE))+"</td><td>"+String(preferences.getUChar(PREF_DHT_PIN))+"</td><td>"+String(dht_read_temperature())+"</td><td>"+String(dht_read_humidity())+"</td><td>";
    html += dht_publish ? "x" : "";
    html += "</td></tr></table>";
  }
  String timestr = "";
  struct tm timeinfo;
  if (wifi_time_read(&timeinfo)) {
    char time_str[100];
    strftime(time_str,sizeof(time_str),"%A %e %B %Y %H:%M:%S",&timeinfo);
    timestr = String(time_str);
  }
  if (input_count > 0 || dht_available()) {
    html += "<form action=\""+String(CONF_WEB_URI_PUBLISH)+"\" method=\"POST\"><p>";
    if (timestr != "") {
      html += timestr + " - ";
    }
    html += "Last Publish: "+String(publish_last_ms)+" - ";
    html += "<button type=\"submit\" class=\"btn btn-primary\">Publish Now</button> ";
    html += "</p></form>";
  } else if (timestr != "") {
    html += "<p>" + timestr + "</p>";
  }
  html += "<p><button type=\"button\" class=\"btn btn-secondary\" onclick=\"location=''\">Refresh</button>";
  html += " <button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='?refresh=30'\">AutoRefresh</button>";
  html += " <button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='"+String(CONF_WEB_URI_CONFIG)+"'\">Configuration</button></p><hr/>";
  html += web_html_footer();
  html += "</body>";
  web_send_page(html_title,html,refresh);
}

void web_rest_handle_input(const Input * input) {
  int8_t pin = input->pin;
  if (pin >= 0 && input->type == DIGITAL_INPUT) {
    web_handle_rest_result(digitalString(digitalReadRuled(pin)));
  } else if (pin >= 0 && input->type == ANALOG_INPUT) {
    web_handle_rest_result(String(analogReadRuled(pin)));
  } else if (pin >= 0 && input->type == MQ2) {
    web_handle_rest_result(String(mq2Read(pin)));
  } else {
    web_handle_rest_notFound();
  }
}

void web_rest_handle_output(const Output * output) {
  String input_value = web_parameter("value");
  bool is_reset = web_parameter("reset").equals("true");
  if (output->type == NONE) {
    web_handle_rest_notFound();
    return;
  }
  if (!web_request_post()) {
    web_handle_rest_result(output_get(output));
    return;
  }
  String error = output_write(output,input_value,is_reset);
  if (error != "") {
    web_send_text(400, "text/plain", error);
    return;
  }
  String redirect_uri = web_header("Referer");
  if (redirect_uri == "") {
    web_send_text(200, "text/plain", "OK");
  } else {
    web_send_redirect(redirect_uri);
  }
}

void web_server_rest_setup(bool dht_available) {
  web_server_register(HTTP_ANY, UriRegex("^\\/rest\\/in([0-9]+)$"), []() {
    int i = web_path_arg(0).toInt();
    if (i <= 0 || i > CONF_SCHEMA_INPUT_COUNT) {
      web_handle_rest_notFound();
      return;
    }
    web_rest_handle_input(&inputs[i-1]);
  });
  web_server_register(HTTP_ANY, UriRegex("^\\/rest\\/out([0-9]+)$"), []() {
    int i = web_path_arg(0).toInt();
    if (i <= 0 || i > CONF_SCHEMA_OUTPUT_COUNT) {
      web_handle_rest_notFound();
      return;
    }
    web_rest_handle_output(&outputs[i-1]);
  });
  web_server_register(HTTP_ANY, UriRegex("^\\/rest\\/in\\/(.+)$"), []() {
    String input_name = web_path_arg(0);
    for (uint8_t i = 0; i < CONF_SCHEMA_INPUT_COUNT; i++) {
      if (inputs[i].type == NO_INPUT) {
        continue;
      }
      if (inputs[i].name == input_name) {
        web_rest_handle_input(&inputs[i]);
        return;
      }
    }
    web_handle_rest_notFound();
  });
  web_server_register(HTTP_ANY, UriRegex("^\\/rest\\/out\\/(.+)$"), []() {
    String input_name = web_path_arg(0);
    for (uint8_t i = 0; i < CONF_SCHEMA_OUTPUT_COUNT; i++) {
      if (outputs[i].type == NONE) {
        continue;
      }
      if (outputs[i].name == input_name) {
        web_rest_handle_output(&outputs[i]);
        return;
      }
    }
    web_handle_rest_notFound();
  });
  if (dht_available) {
    web_server_register(HTTP_ANY, "/rest/dht_temp", []() {
      web_handle_rest_result(String(dht_read_temperature()));
    });
    web_server_register(HTTP_ANY, "/rest/dht_hum", []() {
      web_handle_rest_result(String(dht_read_humidity()));
    });
  }
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
  if (at_interval(input_read_interval_ms,input_read_last_ms)) {
    input_read_last_ms = millis();
    for (uint8_t i = 0; i < CONF_SCHEMA_INPUT_COUNT; i++) {
      if (inputs[i].type == NO_INPUT || !inputs[i].monitored) {
        continue;
      }
      int8_t pin = inputs[i].pin;
      int rs;
      switch (inputs[i].type) {
        case DIGITAL_INPUT:
          if (pin >= 0) {
            rs = digitalRead(pin);
            if (rs != pin_states[pin]) {
              log_d("button %d changed: %d",pin,rs);
              pin_states[pin] = rs;
              on_pin_read(pin,rs,DIGITAL_INPUT,true);
              if (inputs[i].published) {
                item_publish((inputs[i].name).c_str(),digitalString(rs));
              }
            }
          }
          break;
        case ANALOG_INPUT:
          if (pin >= 0) {
            rs = analogRead(pin);
            if (rs != pin_states[pin]) {
              log_d("dimmer %d changed: %d",pin,rs);
              pin_states[pin] = rs;
              on_pin_read(pin,rs,ANALOG_INPUT,true);
            }
          }
          break;
      }
    }
  }
  for (uint8_t i = 0; i < CONF_SCHEMA_OUTPUT_COUNT; i++) {
    if (outputs[i].type == PULSE_1_OUTPUT && outputs[i].pin >= 0) {
      int8_t pin = outputs[i].pin;
      if (pin_states[pin] == HIGH && at_interval(1000,pin_write_last_ms[pin])) {
        digitalWriteState(pin,LOW);
        on_output_write(outputs[i].num);
      }
    }
  }
#ifdef CONF_WEB
  if (!wifi_is_off()) {
    web_server_loop();
    delay(10);
    if (at_interval(publish_interval_ms,publish_last_ms)) {
      publish();
    }
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
  input_read_interval_ms = max(preferences.getULong(PREF_INPUT_READ_INTERVAL),CONF_INPUT_READ_INTERVAL_MIN);
  openhab_rest_uri = preferences.getString(PREF_OPENHAB_REST_URI);
  openhab_bus_item = preferences.getString(PREF_OPENHAB_BUS_ITEM);
  if (openhab_bus_item == "") {
    openhab_bus_item = CONF_OPENHAB_BUS_ITEM;
  }
  publish_interval_ms = preferences.getULong(PREF_PUBLISH_INTERVAL,CONF_PUBLISH_INTERVAL);
  if (publish_interval_ms != 0) {
    publish_interval_ms = max(publish_interval_ms,CONF_PUBLISH_INTERVAL_MIN);
  }
  publish_ip = preferences.getBool(PREF_PUBLISH_IP);
  publish_ssid = preferences.getBool(PREF_PUBLISH_SSID);
  publish_rssi = preferences.getBool(PREF_PUBLISH_RSSI);
  for (uint8_t i = 0; i < CONF_SCHEMA_INPUT_COUNT; i++) {
    inputs[i].num = i+1;
    uint8_t type = preferences.getUChar((PREF_PREFIX_INPUT+String(inputs[i].num)+PREF_PREFIX_INPUT_TYPE).c_str());
    inputs[i].type = (input_type_t)(type & PREF_PREFIX_INPUT_TYPE_BITMASK);
    if (inputs[i].type == NO_INPUT) {
      continue;
    }
    inputs[i].key = PREF_PREFIX_INPUT_KEY+String(inputs[i].num);
    String key_name_str = PREF_PREFIX_INPUT+String(inputs[i].num)+PREF_PREFIX_INPUT_NAME;
    const char * key_name = key_name_str.c_str();
    if (preferences.isKey(key_name)) {
      inputs[i].name = preferences.getString(key_name);
    } else {
      inputs[i].name = "";
    }
    if (inputs[i].name == "") {
      inputs[i].name = inputs[i].key;
    }
    inputs[i].monitored = (type & PREF_PREFIX_INPUT_TYPE_MONITORED) != 0;
    inputs[i].published = (type & PREF_PREFIX_INPUT_TYPE_PUBLISHED) != 0;
    String key_pin_str = PREF_PREFIX_INPUT+String(inputs[i].num)+PREF_PREFIX_INPUT_PIN;
    const char * key_pin = key_pin_str.c_str();
    if (preferences.isKey(key_pin)) {
      inputs[i].pin = preferences.getUChar(key_pin);
    } else {
      inputs[i].pin = -1;
    }
    if (inputs[i].pin < 0) {
      inputs[i].pin = -1;
    } else {
      uint8_t pin = inputs[i].pin;
      pinInit(pin, INPUT);
      if (inputs[i].monitored) {
        switch (inputs[i].type) {
          case DIGITAL_INPUT:
            pin_states[pin] = digitalRead(pin);
            break;
          case ANALOG_INPUT:
            pin_states[pin] = analogRead(pin);
            break;
        }
      }
    }
  }
  for (uint8_t i = 0; i < CONF_SCHEMA_OUTPUT_COUNT; i++) {
    outputs[i].num = i+1;
    uint8_t type = preferences.getUChar((PREF_PREFIX_OUTPUT+String(outputs[i].num)+PREF_PREFIX_OUTPUT_TYPE).c_str());
    outputs[i].type = (output_type_t)(type & PREF_PREFIX_OUTPUT_TYPE_BITMASK);
    if (outputs[i].type == NONE) {
      continue;
    }
    outputs[i].key = PREF_PREFIX_OUTPUT_KEY+String(outputs[i].num);
    const char * key = (outputs[i].key).c_str();
    String key_name_str = PREF_PREFIX_OUTPUT+String(outputs[i].num)+PREF_PREFIX_OUTPUT_NAME;
    const char * key_name = key_name_str.c_str();
    if (preferences.isKey(key_name)) {
      outputs[i].name = preferences.getString(key_name);
    } else {
      outputs[i].name = "";
    }
    if (outputs[i].name == "") {
      outputs[i].name = outputs[i].key;
    }
    outputs[i].stored = (type & PREF_PREFIX_OUTPUT_TYPE_STORED) != 0;
    outputs[i].published = (type & PREF_PREFIX_OUTPUT_TYPE_PUBLISHED) != 0;
    if (!(outputs[i].stored)) {
      preferences.remove(key);
    }
    String key_pin_str = PREF_PREFIX_OUTPUT+String(outputs[i].num)+PREF_PREFIX_OUTPUT_PIN;
    const char * key_pin = key_pin_str.c_str();
    if (preferences.isKey(key_pin)) {
      outputs[i].pin = preferences.getUChar(key_pin);
    } else {
      outputs[i].pin = -1;
    }
    if (outputs[i].pin < 0) {
      outputs[i].pin = -1;
    } else {
      uint8_t pin = outputs[i].pin;
      int value;
      switch (outputs[i].type) {
        case DIGITAL_OUTPUT:
        case PULSE_1_OUTPUT:
          pinInit(pin, OUTPUT);
          pin_output[pin] = &outputs[i];
          value = LOW;
          if (outputs[i].stored) {
            value = preferences.getInt(key);
          }
          digitalWriteState(pin,value,true);
          break;
        case ANALOG_PWM_OUTPUT:
          ledcSetup(channel,CONF_SCHEMA_PWM_FRQ,CONF_SCHEMA_PWM_RES);
          ledcAttachPin(pin, channel);
          pin_channel[pin] = channel++;
          pin_output[pin] = &outputs[i];
          value = 0;
          if (outputs[i].stored) {
            value = preferences.getInt(key);
          }
          analogWriteState(pin,value,true);
          break;
        case ANALOG_FM_OUTPUT:
          ledcSetup(channel,0,CONF_SCHEMA_PWM_RES);
          ledcAttachPin(pin, channel);
          pin_channel[pin] = channel++;
          pin_output[pin] = &outputs[i];
          value = 0;
          if (outputs[i].stored) {
            value = preferences.getInt(key);
          }
          toneState(pin,value,true);
          break;
      }
    }
  }
#ifdef CONF_DHT
  bool dht_available = dht_setup(preferences.getUChar(PREF_DHT_PIN),
    preferences.getUChar(PREF_DHT_TYPE),
    preferences.getULong(PREF_DHT_READ_INTERVAL));
  dht_publish = dht_available && preferences.getBool(PREF_DHT_PUBLISH);
#endif
  rules_setup(preferences.getString(PREF_RULES));
#ifdef CONF_WIFI
  wifi_name = preferences.getString(PREF_WIFI_NAME,CONF_WIFI_NAME);
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
  web_server_rest_setup(dht_available);
  web_server_register(HTTP_POST, CONF_WEB_URI_PUBLISH, []() {
    publish();
    String redirect_uri = web_header("Referer");
    if (redirect_uri == "") {
      redirect_uri = "/";
    }
    web_send_redirect(redirect_uri);
  });
  web_server_register(HTTP_ANY, "/", web_handle_root);
  web_server_begin(wifi_name.c_str());
#endif
}

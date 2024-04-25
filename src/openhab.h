#ifndef MODULO_OPENHAB_H
#define MODULO_OPENHAB_H

#include <WiFiClient.h>
#ifdef PLATFORM_ESP8266
#include <ESP8266HTTPClient.h>
#else
#include <HTTPClient.h>
#endif

class OpenHAB {
private:
  WiFiClient _wifiClient;
  HTTPClient _httpClient;
  String _rest_URI;
public:
  OpenHAB(const String& rest_URI);
  String item_read(const String& item_name);
  bool item_write(const String& item_name, const String& value);
};

OpenHAB::OpenHAB(const String& rest_URI) {
  _rest_URI = rest_URI;
}

String OpenHAB::item_read(const String& item_name) {
  String result = "";
  String request_URI = _rest_URI+"/items/"+item_name+"/state";
  log_d("requesting %s", request_URI.c_str());
  if (!_httpClient.begin(_wifiClient,request_URI)) {
    log_e("request error %s", request_URI.c_str());
  } else {
    int httpCode = _httpClient.GET();
    if (httpCode <= 0) {
      log_e("connection error %d", httpCode);
    } else if (httpCode < 200 || httpCode >= 300) {
      log_e("response error %d : %s", httpCode, _httpClient.getString().c_str());
    } else {
      result = _httpClient.getString();
      log_d("response result : %s", result.c_str());
    }
  }
  _httpClient.end();
  return result;
}

bool OpenHAB::item_write(const String& item_name, const String& value) {
  bool result = false;
  String request_URI = _rest_URI+"/items/"+item_name+"/state";
  log_d("requesting %s", request_URI.c_str());
  if (!_httpClient.begin(_wifiClient,request_URI)) {
    log_e("request error %s", request_URI.c_str());
  } else {
    String payload = value;
    log_d("with payload : ", payload.c_str());
    _httpClient.addHeader("Content-Type", "text/plain; charset=utf-8");
    _httpClient.addHeader("Content-Length", String(payload.length()));
    int httpCode = _httpClient.PUT(payload);
    if (httpCode <= 0) {
      log_e("connection error %d", httpCode);
    } else if (httpCode < 200 || httpCode >= 300) {
      log_e("response error %d : %s", httpCode, _httpClient.getString().c_str());
    } else {
      result = true;
      log_d("response result : %s", _httpClient.getString().c_str());
    }
  }
  _httpClient.end();
  return result;
}

#endif

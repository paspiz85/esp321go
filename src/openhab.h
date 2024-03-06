#ifndef MODULO_OPENHAB_H
#define MODULO_OPENHAB_H

#include "wifi.h"

String openhab_item_state_read(String rest_URI,String item_name) {
  static HTTPClient httpClient;
  String result = "";
  String request_URI = rest_URI+"/items/"+item_name+"/state";
  log_d("requesting %s", request_URI.c_str());
  if (!httpClient.begin(wifiClient,request_URI)) {
    log_e("request error %s", request_URI.c_str());
  } else {
    int httpCode = httpClient.GET();
    if (httpCode <= 0) {
      log_e("connection error %d", httpCode);
    } else if (httpCode < 200 || httpCode >= 300) {
      log_e("response error %d : %s", httpCode, httpClient.getString().c_str());
    } else {
      result = httpClient.getString();
      log_d("response result : %s", result.c_str());
    }
  }
  httpClient.end();
  return result;
}

bool openhab_item_state_write(String rest_URI,String item_name,String value) {
  static HTTPClient httpClient;
  bool result = false;
  String request_URI = rest_URI+"/items/"+item_name+"/state";
  log_d("requesting %s", request_URI.c_str());
  if (!httpClient.begin(wifiClient,request_URI)) {
    log_e("request error %s", request_URI.c_str());
  } else {
    String payload = value;
    log_d("with payload : ", payload.c_str());
    httpClient.addHeader("Content-Type", "text/plain; charset=utf-8");
    httpClient.addHeader("Content-Length", String(payload.length()));
    int httpCode = httpClient.PUT(payload);
    if (httpCode <= 0) {
      log_e("connection error %d", httpCode);
    } else if (httpCode < 200 || httpCode >= 300) {
      log_e("response error %d : %s", httpCode, httpClient.getString().c_str());
    } else {
      result = true;
      log_d("response result : %s", httpClient.getString().c_str());
    }
  }
  httpClient.end();
  return result;
}

#endif

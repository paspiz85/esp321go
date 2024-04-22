#ifndef INCLUDE_WEB_H
#define INCLUDE_WEB_H

/**
 * Contiene variabili, tipi e funzioni per l'uso come Web Server.
 * 
 * @see https://github.com/espressif/arduino-esp32/blob/master/libraries/WebServer/src/WebServer.h
 * @see https://github.com/espressif/arduino-esp32/blob/master/libraries/ESPmDNS/src/ESPmDNS.h
 * @see https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WebServer/src/ESP8266WebServer.h
 * @see https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WebServer/src/ESP8266WebServerSecure.h
 * @see https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266mDNS/src/LEAmDNS.h
 */

#include "base_conf.h"
#include "wifi_utils.h"
#ifdef PLATFORM_ESP8266
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif
#include <uri/UriBraces.h>
#include <uri/UriRegex.h>
#ifdef PLATFORM_ESP8266
#ifdef CONF_WEB_HTTPS
#include <ESP8266WebServerSecure.h>
#endif
#include <ESP8266mDNS.h>
#else
#include <ESPmDNS.h>
#endif

class WebClass {
private:
  String _web_server_hostname;
  uint16_t _http_port = 0;
  uint16_t _https_port = 0;
#ifdef PLATFORM_ESP8266
  ESP8266WebServer * _http_server = NULL;
#else
  WebServer * _http_server = NULL;
#endif
#ifdef CONF_WEB_HTTPS
  BearSSL::ESP8266WebServerSecure * _https_server = NULL;
  void _handleUpgradeHTTPS();
#endif
  bool _mdns_enabled = false;
public:
  void loopToHandleClients();
  bool isRequestSecure();
  bool isRequestMethodPost();
  bool authenticate(const char * username, const char * password);
  void authenticateRequest();
  String getParameter(const String& name);
  String getPathArg(int n);
  String getHeader(const String& name);
  HTTPUpload& getUpload();
  void sendHeader(const String& name, const String& value, bool first = false);
  void sendResponse(int status_code, const String& content_type, const String& text, const String& filename = "");
  void sendResponse(int status_code, const String& content_type, uint8_t chunks_num, const char * chunks[]);
  void sendRedirect(String redirect_uri, const String& message = "", uint16_t refresh = CONF_WEB_REDIRECT_REFRESH_MIN);
  void handle(HTTPMethod method, const Uri &uri, void (*fn)());
  void handleUpload(HTTPMethod method, const Uri &uri, void (*fn)(), void (*ufn)());
  void setupHTTP(const uint16_t port = CONF_WEB_HTTP_PORT);
#ifdef CONF_WEB_HTTPS
  void setupHTTPS(const String& crt = "", const String& key = "", const uint16_t port = CONF_WEB_HTTPS_PORT);
#endif
  void begin(const char * name = "", void (*handle_notFound)() = nullptr);
};

void WebClass::loopToHandleClients() {
  if (!WiFiUtils::isEnabled()) {
    return;
  }
#ifdef PLATFORM_ESP8266
  if (_mdns_enabled) {
    MDNS.update();
  }
#endif
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    _https_server->handleClient();
  }
#endif
  if (_http_server != NULL) {
    _http_server->handleClient();
  }
}

bool WebClass::isRequestSecure() {
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    return true;
  }
#endif
  return false;
}

bool WebClass::isRequestMethodPost() {
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    return _https_server->method() == HTTP_POST;
  }
#endif
  if (_http_server != NULL) {
    return _http_server->method() == HTTP_POST;
  }
  return false;
}

bool WebClass::authenticate(const char * username, const char * password) {
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    return _https_server->authenticate(username, password);
  }
#endif
  if (_http_server != NULL) {
    return _http_server->authenticate(username, password);
  }
  return false;
}

void WebClass::authenticateRequest() {
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    _https_server->requestAuthentication();
    return;
  }
#endif
  if (_http_server != NULL) {
    _http_server->requestAuthentication();
  }
}

String WebClass::getParameter(const String& name) {
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    for (int i = 0; i < _https_server->args(); i++) {
      if (_https_server->argName(i) == name) {
        return _https_server->arg(i);
      }
    }
    return "";
  }
#endif
  if (_http_server != NULL) {
    for (int i = 0; i < _http_server->args(); i++) {
      if (_http_server->argName(i) == name) {
        return _http_server->arg(i);
      }
    }
  }
  return "";
}

String WebClass::getPathArg(int n) {
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    return _https_server->pathArg(n);
  }
#endif
  if (_http_server != NULL) {
    return _http_server->pathArg(n);
  }
  return "";
}

String WebClass::getHeader(const String& name) {
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    return _https_server->header(name);
  }
#endif
  if (_http_server != NULL) {
    return _http_server->header(name);
  }
  return "";
}

HTTPUpload& WebClass::getUpload() {
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    return _https_server->upload();
  }
#endif
  return _http_server->upload();
}

void WebClass::sendHeader(const String& name, const String& value, bool first) {
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    _https_server->sendHeader(name,value,first);
    return;
  }
#endif
  if (_http_server != NULL) {
    _http_server->sendHeader(name,value,first);
  }
}

void WebClass::sendResponse(int status_code, const String& content_type, const String& text, const String& filename) {
  if (filename != "") {
    sendHeader("Content-Disposition", "attachment;filename=\""+filename+"\"");
  }
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    _https_server->send(status_code,content_type,text);
    return;
  }
#endif
  if (_http_server != NULL) {
    _http_server->send(status_code,content_type,text);
  }
}

void WebClass::sendResponse(int status_code, const String& content_type, uint8_t chunks_num, const char * chunks[]) {
  if (chunks_num == 0) {
    sendResponse(status_code,content_type,"");
    return;
  }
  if (chunks_num == 1) {
    sendResponse(status_code,content_type,chunks[0]);
    return;
  }
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    for (int i = 0; i < chunks_num; i++) {
      if (i == 0) {
#ifdef PLATFORM_ESP8266
        _https_server->chunkedResponseModeStart(status_code,content_type);
        _https_server->sendContent(chunks[i]);
#else
        _https_server->send(status_code,content_type,chunks[i]);
#endif
      } else {
        _https_server->sendContent(chunks[i]);
      }
    }
#ifdef PLATFORM_ESP8266
    _https_server->chunkedResponseFinalize();
#endif
  }
#endif
  if (_http_server != NULL) {
    for (int i = 0; i < chunks_num; i++) {
      if (i == 0) {
#ifdef PLATFORM_ESP8266
        _http_server->chunkedResponseModeStart(status_code,content_type);
        _http_server->sendContent(chunks[i]);
#else
        _http_server->send(status_code,content_type,chunks[i]);
#endif
      } else {
        _http_server->sendContent(chunks[i]);
      }
    }
#ifdef PLATFORM_ESP8266
    _http_server->chunkedResponseFinalize();
#endif
  }
}

void WebClass::sendRedirect(String redirect_uri, const String& message, uint16_t refresh ) {
  if (redirect_uri == "") {
    redirect_uri = getHeader("Referer");
    log_d("referer %s", redirect_uri.c_str());
  }
  if (redirect_uri == "") {
    redirect_uri = "/";
  }
  log_d("redirect_uri %s", redirect_uri.c_str());
  if (message == "") {
    sendHeader("Location", redirect_uri, true);
    sendResponse(302,"text/plain","");
    return;
  }
  refresh = max(refresh, CONF_WEB_REDIRECT_REFRESH_MIN);
  sendResponse(200, "text/html", "<html><body>"+message+"</body><script>document.addEventListener('DOMContentLoaded',function(event){setTimeout(function(){location='"+redirect_uri+"'},"+String(refresh)+")})</script></html>");
}

void WebClass::handle(HTTPMethod method, const Uri &uri, void (*fn)()) {
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    _https_server->on(uri, method, fn);
    return;
  }
#endif
  if (_http_server != NULL) {
    _http_server->on(uri, method, fn);
  }
}

void WebClass::handleUpload(HTTPMethod method, const Uri &uri, void (*fn)(), void (*ufn)()) {
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    _https_server->on(uri, method, fn, ufn);
    return;
  }
#endif
  if (_http_server != NULL) {
    _http_server->on(uri, method, fn, ufn);
  }
}

void WebClass::setupHTTP(const uint16_t port) {
  _http_port = port > 0 ? port : CONF_WEB_HTTP_PORT;
#ifdef PLATFORM_ESP8266
  _http_server = new ESP8266WebServer(_http_port);
#else
  _http_server = new WebServer(_http_port);
#endif
}

#ifdef CONF_WEB_HTTPS
void WebClass::setupHTTPS(const String& crt, const String& key, const uint16_t port) {
  _https_port = port > 0 ? port : CONF_WEB_HTTPS_PORT;
  const char * crt_str = crt != "" ? crt.c_str() : CONF_WEB_HTTPS_CERT; 
  const char * key_str = key != "" ? key.c_str() : CONF_WEB_HTTPS_CERT_KEY;
  _https_server = new BearSSL::ESP8266WebServerSecure(_https_port);
  _https_server->getServer().setRSACert(new BearSSL::X509List(crt_str), new BearSSL::PrivateKey(key_str));
}

void WebClass::_handleUpgradeHTTPS() {
  String host = _http_server->header("Host");
  if (host == "") {
    host = _web_server_hostname + ":" + _https_port;
  }
  _http_server->sendHeader("Location", String("https://") + host + "/", true);
  _http_server->send(301, "text/plain", "");
}
#endif

void __web_handle_notFound_default();

void WebClass::begin(const char * name, void (*handle_notFound)()) {
  if (_http_server == NULL) {
    return;
  }
  if (handle_notFound == NULL) {
    handle_notFound = &__web_handle_notFound_default;
  }
  if (strlen(name) > 0 && MDNS.begin(name)) {
    _mdns_enabled = true;
    _web_server_hostname = String(name) + ".local";
    MDNS.addService("http", "tcp", _http_port);
  } else {
    _web_server_hostname = WiFiUtils::getIP();
  }
#ifdef CONF_WEB_HTTPS
  if (_https_server == NULL) {
#endif
    _http_server->onNotFound(handle_notFound);
    const char * headerkeys[] = {"Accept", "Referer"};
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
    _http_server->collectHeaders(headerkeys, headerkeyssize);
    _http_server->begin();
    Serial.print("Ready on http://");
    Serial.print(_web_server_hostname);
    if (_http_port != 80) {
      Serial.print(":");
      Serial.print(_http_port);
    }
#ifdef CONF_WEB_HTTPS
  } else {
    _http_server->onNotFound([this]() { this->_handleUpgradeHTTPS(); });
    _http_server->begin();
    _https_server->onNotFound(handle_notFound);
    const char * headerkeys[] = {"Accept", "Referer"};
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
    _https_server->collectHeaders(headerkeys, headerkeyssize);
    _https_server->begin();
    if (_mdns_enabled) {
      MDNS.addService("https", "tcp", _https_port);
    }
    Serial.print("Ready on https://");
    Serial.print(_web_server_hostname);
    if (_https_port != 443) {
      Serial.print(":");
      Serial.print(_https_port);
    }
  }
#endif
  Serial.println("/");
}

WebClass Web;

void __web_handle_notFound_default() {
  Web.sendResponse(404, "text/plain", "Not Found");
}

#endif

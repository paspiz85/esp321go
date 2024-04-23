#ifndef INCLUDE_WEB_PLATFORM_H
#define INCLUDE_WEB_PLATFORM_H

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

class WebPlatform {
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
#ifdef CONF_WEB_HTTPS
  WebPlatform(const uint16_t http_port = CONF_WEB_HTTP_PORT, const uint16_t https_port = CONF_WEB_HTTPS_PORT, const String& crt = "", const String& key = "");
#else
  WebPlatform(const uint16_t port = CONF_WEB_HTTP_PORT);
#endif
  void loopToHandleClients();
  bool isRequestSecure();
  bool isRequestMethodPost();
  bool authenticate(const char * username, const char * password);
  void authenticateRequest();
  String getParameter(const String& name);
  String getPathArg(int n);
  String getHeader(const String& name);
  void sendHeader(const String& name, const String& value, bool first = false);
  void cacheControl(bool value);
  void cookieCreate(const String& name, const String& value, unsigned long max_age);
  void cookieDelete(const String& name);
  String cookieRead(const String& name);
  HTTPUpload& getUpload();
  void sendResponse(int status_code, const String& content_type, const String& text, const String& filename = "");
  void sendResponse(int status_code, const String& content_type, uint8_t chunks_num, const char * chunks[]);
  void sendRedirect(String redirect_uri, const String& message = "", uint16_t refresh = CONF_WEB_REDIRECT_REFRESH_MIN);
  void handle(HTTPMethod method, const Uri &uri, std::function<void(void)> fn);
  void handleUpload(HTTPMethod method, const Uri &uri, std::function<void(void)> fn, std::function<void(void)> ufn);
  virtual void begin(const char * name = "", void (*handle_notFound)() = nullptr);
};

void WebPlatform::loopToHandleClients() {
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

bool WebPlatform::isRequestSecure() {
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    return true;
  }
#endif
  return false;
}

bool WebPlatform::isRequestMethodPost() {
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

bool WebPlatform::authenticate(const char * username, const char * password) {
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

void WebPlatform::authenticateRequest() {
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

String WebPlatform::getParameter(const String& name) {
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

String WebPlatform::getPathArg(int n) {
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

String WebPlatform::getHeader(const String& name) {
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

void WebPlatform::sendHeader(const String& name, const String& value, bool first) {
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

void WebPlatform::cacheControl(bool value) {
  if (!value) {
    sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    sendHeader("Pragma", "no-cache");
    sendHeader("Expires", "0");
  }
}

void WebPlatform::cookieCreate(const String& name, const String& value, unsigned long max_age) {
  String cookieHeader = name + "=" + value;
  if (max_age > 0) {
    cookieHeader += "; max-age=";
    cookieHeader += String(max_age).c_str();
  }
  sendHeader("Set-Cookie", cookieHeader);
}

void WebPlatform::cookieDelete(const String& name) {
  String cookieHeader = name + "=; expires=Thu, 01 Jan 1970 00:00:00 GMT";
  sendHeader("Set-Cookie", cookieHeader);
}

String WebPlatform::cookieRead(const String& name) {
  String cookieHeader = getHeader("Cookie");
  if (cookieHeader == "") {
    return "";
  }
  int inizio = cookieHeader.indexOf(name + "=");
  if (inizio == -1) {
    return "";
  }
  if (inizio > 0 && cookieHeader.charAt(inizio - 1) != ' ') {
    inizio = cookieHeader.indexOf(" " + name + "=");
    if (inizio == -1) {
      return "";
    }
  }
  inizio += name.length() + 1;
  int fine = cookieHeader.indexOf(';', inizio);
  if (fine == -1) {
    fine = cookieHeader.length();
  }
  return cookieHeader.substring(inizio, fine);
}

HTTPUpload& WebPlatform::getUpload() {
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    return _https_server->upload();
  }
#endif
  return _http_server->upload();
}

void WebPlatform::sendResponse(int status_code, const String& content_type, const String& text, const String& filename) {
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

void WebPlatform::sendResponse(int status_code, const String& content_type, uint8_t chunks_num, const char * chunks[]) {
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

void WebPlatform::sendRedirect(String redirect_uri, const String& message, uint16_t refresh ) {
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

void WebPlatform::handle(HTTPMethod method, const Uri &uri, std::function<void(void)> fn) {
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

void WebPlatform::handleUpload(HTTPMethod method, const Uri &uri, std::function<void(void)> fn, std::function<void(void)> ufn) {
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

#ifdef CONF_WEB_HTTPS
void WebPlatform::_handleUpgradeHTTPS() {
  String host = _http_server->header("Host");
  if (host == "") {
    host = _web_server_hostname + ":" + _https_port;
  }
  _http_server->sendHeader("Location", String("https://") + host + "/", true);
  _http_server->send(301, "text/plain", "");
}

WebPlatform::WebPlatform(const uint16_t http_port, const uint16_t https_port, const String& crt, const String& key) {
#else
WebPlatform::WebPlatform(const uint16_t http_port) {
#endif
  _http_port = http_port > 0 ? http_port : CONF_WEB_HTTP_PORT;
#ifdef PLATFORM_ESP8266
  _http_server = new ESP8266WebServer(_http_port);
#else
  _http_server = new WebServer(_http_port);
#endif
#ifdef CONF_WEB_HTTPS
  if (_https_port == 0 && crt == "" && key == "") {
    return;
  }
  _https_port = https_port > 0 ? https_port : CONF_WEB_HTTPS_PORT;
  const char * crt_str = crt != "" ? crt.c_str() : CONF_WEB_HTTPS_CERT; 
  const char * key_str = key != "" ? key.c_str() : CONF_WEB_HTTPS_CERT_KEY;
  _https_server = new BearSSL::ESP8266WebServerSecure(_https_port);
  _https_server->getServer().setRSACert(new BearSSL::X509List(crt_str), new BearSSL::PrivateKey(key_str));
#endif
}

void WebPlatform::begin(const char * name, void (*handle_notFound)()) {
  if (_http_server == NULL) {
    return;
  }
  if (strlen(name) > 0 && MDNS.begin(name)) {
    _mdns_enabled = true;
    _web_server_hostname = String(name) + ".local";
    MDNS.addService("http", "tcp", _http_port);
  } else {
    _web_server_hostname = WiFiUtils::getIP();
  }
  const char * header_keys[] = {"Accept", "Referer"};
  size_t header_keys_size = sizeof(header_keys) / sizeof(char*);
#ifdef CONF_WEB_HTTPS
  if (_https_server == NULL) {
#endif
    if (handle_notFound == NULL) {
      _http_server->onNotFound([this]() { this->sendResponse(404, "text/plain", "Not Found"); });
    } else {
      _http_server->onNotFound(handle_notFound);
    }
    _http_server->collectHeaders(header_keys, header_keys_size);
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
    if (handle_notFound == NULL) {
      _https_server->onNotFound([this]() { this->sendResponse(404, "text/plain", "Not Found"); });
    } else {
      _https_server->onNotFound(handle_notFound);
    }
    _https_server->collectHeaders(header_keys, header_keys_size);
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

#endif

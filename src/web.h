#ifndef INCLUDE_WEB_H
#define INCLUDE_WEB_H

/**
 * Contiene variabili, tipi e funzioni per l'uso come Web Server.
 * 
 * @see https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WebServer/src/ESP8266WebServer.h
 * @see https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266mDNS/src/LEAmDNS.h
 */

#include "base_conf.h"
#include "wifi_utils.h"
#include <ESP8266WebServer.h>
#include <uri/UriBraces.h>
#include <uri/UriRegex.h>
#include <ESP8266mDNS.h>

class WebClass {
public:
  void loopToHandleClients();
  bool isRequestMethodPost();
  bool authenticate(const char * username, const char * password);
  void authenticateRequest();
  String getParameter(String name);
  String getPathArg(int n);
  String getHeader(String name);
  HTTPUpload& getUpload();
  void sendRedirect(String redirect_uri, String message = "", uint16_t refresh = CONF_WEB_REDIRECT_REFRESH_MIN);
  void sendResponse(int status_code, String content_type, const char * text);
  void sendResponse(int status_code, String content_type, String text);
  void sendFile(String content_type, String filename, String text);
  void handle(HTTPMethod method, const Uri &uri, ESP8266WebServer::THandlerFunction fn);
  void handleUpload(HTTPMethod method, const Uri &uri, ESP8266WebServer::THandlerFunction fn, ESP8266WebServer::THandlerFunction ufn);
  void setupHTTP(const uint16_t port = CONF_WEB_HTTP_PORT);
#ifdef CONF_WEB_HTTPS
  bool setupHTTPS(String crt = "", String key = "", const uint16_t port = CONF_WEB_HTTPS_PORT);
#endif
  void begin(const char * name = "", void (*handle_notFound)() = nullptr);
private:
  String _web_server_hostname;
  uint16_t _http_port = 0;
  uint16_t _https_port = 0;
  ESP8266WebServer * _http_server = NULL;
#ifdef CONF_WEB_HTTPS
  ESP8266WebServer * _https_server = NULL;
#endif
  bool _mdns_enabled = false;
};

void WebClass::loopToHandleClients() {
  if (_mdns_enabled) {
    MDNS.update();
  }
  if (!WiFiUtils.isEnabled()) {
    return;
  }
  if (_http_server != NULL) {
    _http_server->handleClient();
  }
}

bool WebClass::isRequestMethodPost() {
  if (_http_server != NULL) {
    return _http_server->method() == HTTP_POST;
  }
  return false;
}

bool WebClass::authenticate(const char * username, const char * password) {
  if (_http_server != NULL) {
    return _http_server->authenticate(username, password);
  }
  return false;
}

void WebClass::authenticateRequest() {
  if (_http_server != NULL) {
    _http_server->requestAuthentication();
  }
}

String WebClass::getParameter(String name) {
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
  if (_http_server != NULL) {
    return _http_server->pathArg(n);
  }
  return "";
}

String WebClass::getHeader(String name) {
  if (_http_server != NULL) {
    return _http_server->header(name);
  }
  return "";
}

HTTPUpload& WebClass::getUpload() {
  return _http_server->upload();
}

void WebClass::sendRedirect(String redirect_uri, String message, uint16_t refresh ) {
  if (redirect_uri == "") {
    redirect_uri = getHeader("Referer");
    log_d("referer %s", redirect_uri.c_str());
  }
  if (redirect_uri == "") {
    redirect_uri = "/";
  }
  log_d("redirect_uri %s", redirect_uri.c_str());
  if (_http_server != NULL) {
    if (message == "") {
      _http_server->sendHeader("Location", redirect_uri, true);
      _http_server->sendHeader("Connection", "close");
      _http_server->send(302,"text/plain","");
      return;
    }
    refresh = max(refresh, CONF_WEB_REDIRECT_REFRESH_MIN);
    _http_server->sendHeader("Connection", "close");
    _http_server->send(200, "text/html", "<html><body>"+message+"</body><script>document.addEventListener('DOMContentLoaded',function(event){setTimeout(function(){location='"+redirect_uri+"'},"+String(refresh)+")})</script></html>");
  }
}

void WebClass::sendResponse(int status_code, String content_type, const char * text) {
  if (_http_server != NULL) {
    _http_server->send(status_code,content_type,text);
  }
}

void WebClass::sendResponse(int status_code, String content_type, String text) {
  if (_http_server != NULL) {
    _http_server->send(status_code,content_type,text);
  }
}

void WebClass::sendFile(String content_type, String filename, String text) {
  if (_http_server != NULL) {
    _http_server->sendHeader("Content-Disposition", "attachment;filename=\""+filename+"\"");
    _http_server->send(200,content_type,text);
  }
}

void WebClass::handle(HTTPMethod method, const Uri &uri, ESP8266WebServer::THandlerFunction fn) {
  if (_http_server != NULL) {
    _http_server->on(uri, method, fn);
  }
}

void WebClass::handleUpload(HTTPMethod method, const Uri &uri, ESP8266WebServer::THandlerFunction fn, ESP8266WebServer::THandlerFunction ufn) {
  if (_http_server != NULL) {
    _http_server->on(uri, method, fn, ufn);
  }
}

void WebClass::setupHTTP(const uint16_t port) {
  _http_port = port > 0 ? port : CONF_WEB_HTTP_PORT;
  _http_server = new ESP8266WebServer(_http_port);
}

#ifdef CONF_WEB_HTTPS
void WebClass::setupHTTPS(String crt, String key, const uint16_t port) {
  _https_port = port > 0 ? port : CONF_WEB_HTTPS_PORT;
  const char * crt_str = crt != "" ? crt.c_str() : CONF_WEB_HTTPS_CERT; 
  const char * key_str = key != "" ? key.c_str() : CONF_WEB_HTTPS_CERT_KEY;
  _https_server = new BearSSL::ESP8266WebServerSecure(_https_port);
  _https_server->getServer().setRSACert(new BearSSL::X509List(crt_str), new BearSSL::PrivateKey(key_str));
}

void __web_handleHTTPSupgrade() {
  String host = web_header("Host");
  if (host == "") {
    host = _web_server_hostname + ":" + _https_port;
  }
  http_server->sendHeader("Location", String("https://") + host + "/", true);
  http_server->send(301, "text/plain", "");
}

void __web_handle_https_upgrade();
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
    _web_server_hostname = WiFiUtils.getIP();
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
    _http_server->onNotFound(__web_handle_https_upgradefgdgdfgdf);
    _http_server->begin();
    MDNS.addService("https", "tcp", _https_port);
    _https_server->onNotFound(handle_notFound);
    const char * headerkeys[] = {"Accept", "Referer"};
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
    _https_server->collectHeaders(headerkeys, headerkeyssize);
    _https_server->begin();
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

#ifdef CONF_WEB_HTTPS
void __web_handle_https_upgrade() {
  String host = web_header("Host");
  if (host == "") {
    host = _web_server_hostname + ":" + _https_port;
  }
  http_server->sendHeader("Location", String("https://") + host + "/", true);
  http_server->send(301, "text/plain", "");
}
#endif

#endif

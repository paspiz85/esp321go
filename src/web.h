#ifndef INCLUDE_WEB_H
#define INCLUDE_WEB_H

/**
 * Contiene variabili, tipi e funzioni per l'uso come Web Server.
 * 
 * @see https://github.com/fhessel/esp32_https_server/blob/master/examples/HTTPS-and-HTTP/HTTPS-and-HTTP.ino
 * @see https://github.com/espressif/arduino-esp32/blob/master/libraries/ESPmDNS/src/ESPmDNS.h
 */

#include "base_conf.h"
#include "wifi_utils.h"
#include <HTTPServer.hpp>
#include <HTTPBodyParser.hpp>
#include <HTTPURLEncodedBodyParser.hpp>
#include <HTTPMultipartBodyParser.hpp>
#include <ESPmDNS.h>
#ifdef CONF_WEB_HTTPS
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#endif

#define HTTP_ANY     ""
#define HTTP_GET     "GET"
#define HTTP_POST    "POST"

using httpsserver::HTTPRequest;
using httpsserver::HTTPResponse;
using httpsserver::HTTPBodyParser;
using httpsserver::HTTPURLEncodedBodyParser;
using httpsserver::HTTPMultipartBodyParser;
using httpsserver::HTTPSCallbackFunction;

class WebClass {
public:
  void loopToHandleClients();
  bool isRequestMethodPost(HTTPRequest * req);
  bool authenticate(HTTPRequest * req, const char * username, const char * password);
  void authenticateRequest(HTTPRequest * req, HTTPResponse * res);
  void cacheControl(HTTPRequest * req, HTTPResponse * res, bool value);
  void cookieCreate(HTTPRequest * req, HTTPResponse * res, const char * name, const char * value, unsigned long max_age = 0);
  void cookieDelete(HTTPRequest * req, HTTPResponse * res, const char * name);
  String cookieRead(HTTPRequest * req, HTTPResponse * res, const char * name);
  String getParameter(HTTPBodyParser * parser);
  String getParameter(HTTPRequest * req, const char * name);
  void sendRedirect(HTTPRequest * req, HTTPResponse * res, String redirect_uri, String message = "", uint16_t refresh = CONF_WEB_REDIRECT_REFRESH_MIN);
  void sendData(HTTPRequest * req, HTTPResponse * res, JSONVar data);
  void sendFile(HTTPRequest * req, HTTPResponse * res, String content_type, String filename, String text);
  void handle(const std::string &method, const std::string &path, const HTTPSCallbackFunction * callback);
  bool setupHTTP(const uint16_t port = CONF_WEB_HTTP_PORT);
#ifdef CONF_WEB_HTTPS
  bool setupHTTPS(String crt = "", String key = "", const uint16_t port = CONF_WEB_HTTPS_PORT);
#endif
  void begin(const char * name = "", void (*handle_notFound)(HTTPRequest * req, HTTPResponse * res) = nullptr, bool secure = false);
private:
  String _web_server_hostname;
  uint16_t _http_port = 0;
  uint16_t _https_port = 0;
  httpsserver::HTTPServer * _http_server = NULL;
#ifdef CONF_WEB_HTTPS
  httpsserver::HTTPSServer * _https_server = NULL;
#endif
  bool _https_server_redirect = false;
  bool _mdns_enabled = false;
  void _handleResourceNode(httpsserver::ResourceNode * node);
};

void WebClass::loopToHandleClients() {
  if (!WiFiUtils.isEnabled()) {
    return;
  }
  if (_http_server != NULL) {
    _http_server->loop();
  }
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    _https_server->loop();
  }
#endif
}

bool WebClass::isRequestMethodPost(HTTPRequest * req) {
  return req->getMethod().compare("POST") == 0;
}

bool WebClass::authenticate(HTTPRequest * req, const char * username, const char * password) {
  log_d("authentication check"); 
  std::string reqUsername = req->getBasicAuthUser();
  std::string reqPassword = req->getBasicAuthPassword();
  if (reqUsername.length() <= 0 || reqPassword.length() <= 0) {
    log_d("authentication empty");
    return false;
  }
  std::transform(reqUsername.begin(), reqUsername.end(), reqUsername.begin(), ::tolower);
  bool result = reqUsername.compare(username) == 0 && reqPassword.compare(password) == 0;
  if (result) {
    log_i("authenticated %s",reqUsername.c_str());
  } else {
    log_d("authentication failed");
  }
  return result;
}

void WebClass::authenticateRequest(HTTPRequest * req, HTTPResponse * res) {
  log_d("authentication needed"); 
  req->discardRequestBody();
  res->setStatusCode(401);
  res->setStatusText("Unauthorized");
  res->setHeader("Content-Type", "text/plain");
  res->setHeader("WWW-Authenticate", "Basic realm=\"Privileged area\"");
  res->println("401. Unauthorized");
  res->finalize();
}

void WebClass::cacheControl(HTTPRequest * req, HTTPResponse * res, bool value) {
  if (!value) {
    res->setHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    res->setHeader("Pragma", "no-cache");
    res->setHeader("Expires", "0");
  }
}

void WebClass::cookieCreate(HTTPRequest * req, HTTPResponse * res, const char * name, const char * value, unsigned long max_age) {
  std::string cookieHeader = std::string(name) + "=" + value;
  if (max_age > 0) {
    cookieHeader += "; max-age=";
    cookieHeader += String(max_age).c_str();
  }
  res->setHeader("Set-Cookie", cookieHeader);
}

void WebClass::cookieDelete(HTTPRequest * req, HTTPResponse * res, const char * name) {
  std::string cookieHeader = std::string(name) + "=; expires=Thu, 01 Jan 1970 00:00:00 GMT";
  res->setHeader("Set-Cookie", cookieHeader);
}

String WebClass::cookieRead(HTTPRequest * req, HTTPResponse * res, const char * name) {
  std::string cookieHeader = req->getHeader("Cookie");
  if (cookieHeader.empty()) {
    return "";
  }
  size_t i = cookieHeader.find(std::string(name) + "=");
  if (i == std::string::npos) {
    return "";
  }
  size_t j = cookieHeader.find(';', i);
  if (j == std::string::npos) {
    j = cookieHeader.length();
  }
  return String(cookieHeader.substr(i + strlen(name) + 1, j - i - strlen(name) - 1).c_str());
}

String WebClass::getParameter(HTTPBodyParser * parser) {
  std::string str = "";
  while (!parser->endOfField()) {
    char buf[513];
    size_t curr_len = parser->read((byte *)buf, sizeof(buf)-1);
    Serial.println(std::string(buf, curr_len).c_str());
    str = str + std::string(buf, curr_len);
  }
  return String(str.c_str());
}

String WebClass::getParameter(HTTPRequest * req, const char * param_name) {
  std::string str;
  auto params = req->getParams();
  if (params->getQueryParameter(param_name, str)) {
    return String(str.c_str());
  }
  return "";
}

void WebClass::sendRedirect(HTTPRequest * req, HTTPResponse * res, String redirect_uri, String message, uint16_t refresh ) {
  if (redirect_uri == "") {
    std::string header = req->getHeader("Referer");
    if (header.length() > 0) {
      redirect_uri = String(header.c_str());
      log_d("referer %s", redirect_uri.c_str());
    }
  }
  if (redirect_uri == "") {
    redirect_uri = "/";
  }
  log_d("redirect_uri %s", redirect_uri.c_str());
  req->discardRequestBody();
  if (message == "") {
    res->setStatusCode(302);
    res->setHeader("Location", redirect_uri.c_str());
    res->setHeader("Connection", "close");
    res->setHeader("Content-Type", "text/plain");
    res->println("");
    res->finalize();
    return;
  }
  refresh = max(refresh, CONF_WEB_REDIRECT_REFRESH_MIN);
  res->setHeader("Connection", "close");
  res->printf("<html><body>%s</body><script>document.addEventListener('DOMContentLoaded',function(event){setTimeout(function(){location='%s'},%d)})</script></html>", message.c_str(), redirect_uri.c_str(), refresh);
  res->finalize();
}

void WebClass::sendData(HTTPRequest * req, HTTPResponse * res, JSONVar data) {
  req->discardRequestBody();
  res->setHeader("Content-Type", "application/json");
  res->println(JSON.stringify(data));
  res->finalize();
}

void WebClass::sendFile(HTTPRequest * req, HTTPResponse * res, String content_type, String filename, String text) {
  req->discardRequestBody();
  res->setHeader("Content-Disposition", ("attachment;filename=\""+filename+"\"").c_str());
  res->setHeader("Content-Type", content_type.c_str());
  res->println(text);
  res->finalize();
}

void WebClass::_handleResourceNode(httpsserver::ResourceNode * node) {
  if (_http_server != NULL && !_https_server_redirect) {
    _http_server->registerNode(node);
  }
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    _https_server->registerNode(node);
  }
#endif
}

void WebClass::handle(const std::string &method, const std::string &path, const HTTPSCallbackFunction * callback) {
  if (method == HTTP_ANY) {
    _handleResourceNode(new httpsserver::ResourceNode(path, HTTP_GET, callback));
    _handleResourceNode(new httpsserver::ResourceNode(path, HTTP_POST, callback));
  } else {
    _handleResourceNode(new httpsserver::ResourceNode(path, method, callback));
  }
}

bool WebClass::setupHTTP(const uint16_t port) {
  _http_port = port > 0 ? port : CONF_WEB_HTTP_PORT;
  log_d("initializing http_server on port %d", _http_port);
  _http_server = new httpsserver::HTTPServer(_http_port);
  return true;
}

#ifdef CONF_WEB_HTTPS
bool WebClass::setupHTTPS(String crt, String key, const uint16_t port) {
  if (crt == "" || key == "") {
    crt = CONF_WEB_HTTPS_CERT;
    key = CONF_WEB_HTTPS_CERT_KEY;
  }
  httpsserver::SSLCert * https_cert;
  if (crt == "" || key == "") {
    log_d("create self-signed cert");
    https_cert = new httpsserver::SSLCert();
    int res = createSelfSignedCert(*https_cert, httpsserver::KEYSIZE_1024, "CN=esp32.local,O=acme,C=IT");
    if (res != 0) {
      log_e("create self-signed cert fails");
      return false;
    }
  } else {
    log_d("loading cert");
    size_t crt_len;
    size_t key_len;
    unsigned char * cert_crt = base64_decode(reinterpret_cast<const unsigned char *>(crt.c_str()),crt.length(),&crt_len);
    unsigned char * cert_key = base64_decode(reinterpret_cast<const unsigned char *>(key.c_str()),key.length(),&key_len);
    https_cert = new httpsserver::SSLCert(
      cert_crt, crt_len,
      cert_key, key_len
    );
  }
  _https_port = port > 0 ? port : CONF_WEB_HTTPS_PORT;
  log_d("initializing https_server on port %d", _https_port);
  _https_server = new httpsserver::HTTPSServer(https_cert,_https_port);
  return true;
}
#endif

void WebClass::begin(const char * name, void (*handle_notFound)(HTTPRequest * req, HTTPResponse * res), bool secure) {
  if (handle_notFound == NULL) {
    handle_notFound = [](HTTPRequest * req, HTTPResponse * res) {
      req->discardRequestBody();
      res->setStatusCode(404);
      res->setHeader("Content-Type", "text/plain");
      res->println("Not Found");
      res->finalize();
    };
  }
  httpsserver::ResourceNode * node404 = new httpsserver::ResourceNode("", HTTP_GET, handle_notFound);
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    _https_server_redirect = secure;
  }
  if (_https_server_redirect) {
    httpsserver::ResourceNode * nodeHttpsRedirect = new httpsserver::ResourceNode("", HTTP_ANY, [](HTTPRequest * req, HTTPResponse * res) {
      std::string host = req->getHeader("Host");
      if (host == "") {
        host = CONF_WEB_HTTPS_NAME;
      }
      std::string redirect_uri = "https://" + host + req->getRequestString();
      req->discardRequestBody();
      res->setStatusCode(302);
      res->setHeader("Location", redirect_uri.c_str());
      res->setHeader("Connection", "close");
      res->setHeader("Content-Type", "text/plain");
      res->println("");
      res->finalize();
    });
    _http_server->setDefaultNode(nodeHttpsRedirect);
  } else {
#endif
    _http_server->setDefaultNode(node404);
#ifdef CONF_WEB_HTTPS
  }
#endif
  _http_server->start();
  String protocol;
  uint16_t port;
  uint16_t port_default;
#ifdef CONF_WEB_HTTPS
  if (_https_server == NULL) {
#endif
    protocol = "http";
    port = _http_port;
    port_default = 80;
#ifdef CONF_WEB_HTTPS
  } else {
    protocol = "https";
    port = _https_port;
    port_default = 443;
    _https_server->setDefaultNode(node404);
    _https_server->start();
  }
#endif
  Serial.print("Ready on ");
  Serial.print(protocol);
  Serial.print("://");
  if (MDNS.begin(name)) {
    MDNS.addService(protocol, "tcp", port);
    Serial.print(name);
    Serial.print(".local");
  } else {
    Serial.print(WiFiUtils.getIP());
  }
  if (port != port_default) {
    Serial.print(":");
    Serial.print(port);
  }
  Serial.println("/");
}

WebClass Web;

#endif

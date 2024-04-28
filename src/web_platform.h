#pragma once

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

const PROGMEM std::string   HTTP_ANY              = "";
const PROGMEM std::string   HTTP_GET              = "GET";
const PROGMEM std::string   HTTP_POST              = "POST";

using httpsserver::HTTPRequest;
using httpsserver::HTTPResponse;
using httpsserver::HTTPBodyParser;
using httpsserver::HTTPURLEncodedBodyParser;
using httpsserver::HTTPMultipartBodyParser;
using httpsserver::HTTPSCallbackFunction;

class WebPlatform {
private:
  String _web_server_hostname;
  uint16_t _http_port = 0;
  uint16_t _https_port = 0;
  httpsserver::HTTPServer* _http_server = NULL;
#ifdef CONF_WEB_HTTPS
  httpsserver::HTTPSServer* _https_server = NULL;
#endif
  bool _https_server_redirect = false;
  bool _mdns_enabled = false;
  void _handleResourceNode(httpsserver::ResourceNode* node);
public:
#ifdef CONF_WEB_HTTPS
  WebPlatform(const uint16_t http_port = CONF_WEB_HTTP_PORT, const uint16_t https_port = CONF_WEB_HTTPS_PORT, String crt = "", String key = "");
#else
  WebPlatform(const uint16_t port = CONF_WEB_HTTP_PORT);
#endif
  void loopToHandleClients();
  void handle(const std::string& method, const String& path, void (*callback)(HTTPRequest*,HTTPResponse*));
  virtual void begin(const char* name = "", bool secure = false, void (*handle_notFound)(HTTPRequest*,HTTPResponse*) = NULL);
};

void WebPlatform::loopToHandleClients() {
  if (!WiFiUtils::isEnabled()) {
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

bool web_isRequestMethodPost(HTTPRequest* req) {
  return req->getMethod().compare("POST") == 0;
}

bool web_authenticate(HTTPRequest* req, const char* username, const char* password) {
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

void web_authenticateRequest(HTTPRequest* req, HTTPResponse* res) {
  log_d("authentication needed"); 
  req->discardRequestBody();
  res->setStatusCode(401);
  res->setStatusText("Unauthorized");
  res->setHeader("Content-Type", "text/plain");
  res->setHeader("WWW-Authenticate", "Basic realm=\"Privileged area\"");
  res->println("401. Unauthorized");
  res->finalize();
}

void web_cacheControl(HTTPRequest* req, HTTPResponse* res, bool value) {
  if (!value) {
    res->setHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    res->setHeader("Pragma", "no-cache");
    res->setHeader("Expires", "0");
  }
}

void web_cookieCreate(HTTPRequest* req, HTTPResponse* res, const String& name, const String& value, unsigned long max_age) {
  String cookieHeader = name + "=" + value;
  if (max_age > 0) {
    cookieHeader += "; max-age=";
    cookieHeader += max_age;
  }
  res->setHeader("Set-Cookie", std::string(cookieHeader.c_str()));
}

void web_cookieDelete(HTTPRequest* req, HTTPResponse* res, const String& name) {
  String cookieHeader = name + "=; expires=Thu, 01 Jan 1970 00:00:00 GMT";
  res->setHeader("Set-Cookie", std::string(cookieHeader.c_str()));
}

String web_cookieRead(HTTPRequest* req, const String& name) {
  std::string cookieHeader = req->getHeader("Cookie");
  if (cookieHeader.empty()) {
    return "";
  }
  std::string cookieName = std::string(name.c_str());
  int inizio = cookieHeader.find(cookieName + "=");
  if (inizio == std::string::npos) {
    return "";
  }
  if (inizio > 0 && cookieHeader[inizio - 1] != ' ') {
    inizio = cookieHeader.find(" " + cookieName + "=");
    if (inizio == std::string::npos) {
      return "";
    }
  }
  inizio += name.length() + 1;
  int fine = cookieHeader.find(';', inizio);
  if (fine == std::string::npos) {
    fine = cookieHeader.length();
  }
  return String(cookieHeader.substr(inizio, fine).c_str());
}

String web_getParameter(HTTPBodyParser* parser) {
  std::string str = "";
  while (!parser->endOfField()) {
    char buf[513];
    size_t curr_len = parser->read((byte*)buf, sizeof(buf)-1);
    Serial.println(std::string(buf, curr_len).c_str());
    str = str + std::string(buf, curr_len);
  }
  return String(str.c_str());
}

String web_getParameter(HTTPRequest* req, const char* param_name) {
  std::string str;
  auto params = req->getParams();
  if (params->getQueryParameter(param_name, str)) {
    return String(str.c_str());
  }
  return "";
}

void web_sendResponse(HTTPRequest* req, HTTPResponse* res, int status_code, const String& content_type, const String& text, const String& filename = "") {
  req->discardRequestBody();
  res->setStatusCode(status_code);
  if (filename != null) {
    res->setHeader("Content-Disposition", ("attachment;filename=\""+filename+"\"").c_str());
  }
  res->setHeader("Content-Type", content_type.c_str());
  res->println(text);
  res->finalize();
}

void web_sendRedirect(HTTPRequest* req, HTTPResponse* res, String redirect_uri, const String& message = "", uint16_t refresh = CONF_WEB_REDIRECT_REFRESH_MIN) {
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

void WebPlatform::_handleResourceNode(httpsserver::ResourceNode* node) {
  if (_http_server != NULL && !_https_server_redirect) {
    _http_server->registerNode(node);
  }
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    _https_server->registerNode(node);
  }
#endif
}

void WebPlatform::handle(const std::string& method, const String& path, void (*callback)(HTTPRequest*,HTTPResponse*)) {
  std::string p = std::string(path.c_str());
  if (method == HTTP_ANY) {
    _handleResourceNode(new httpsserver::ResourceNode(p, HTTP_GET, callback));
    _handleResourceNode(new httpsserver::ResourceNode(p, HTTP_POST, callback));
  } else {
    _handleResourceNode(new httpsserver::ResourceNode(p, method, callback));
  }
}

#ifdef CONF_WEB_HTTPS
WebPlatform::WebPlatform(const uint16_t http_port, const uint16_t https_port, String crt, String key) {
#else
WebPlatform::WebPlatform(const uint16_t http_port) {
#endif
  _http_port = http_port > 0 ? http_port : CONF_WEB_HTTP_PORT;
  log_d("initializing http_server on port %d", _http_port);
  _http_server = new httpsserver::HTTPServer(_http_port);
#ifdef CONF_WEB_HTTPS
  if (_https_port == 0 && crt == "" && key == "") {
    return;
  }
  if (crt == "" || key == "") {
    crt = CONF_WEB_HTTPS_CERT;
    key = CONF_WEB_HTTPS_CERT_KEY;
  }
  httpsserver::SSLCert* https_cert;
  if (crt == "" || key == "") {
    log_d("create self-signed cert");
    https_cert = new httpsserver::SSLCert();
    int res = createSelfSignedCert(*https_cert, httpsserver::KEYSIZE_1024, "CN=esp32.local,O=acme,C=IT");
    if (res != 0) {
      log_e("create self-signed cert fails");
      return;
    }
  } else {
    log_d("loading cert");
    size_t crt_len;
    size_t key_len;
    unsigned char* cert_crt = base64_decode(reinterpret_cast<const unsigned char*>(crt.c_str()),crt.length(),&crt_len);
    unsigned char* cert_key = base64_decode(reinterpret_cast<const unsigned char*>(key.c_str()),key.length(),&key_len);
    https_cert = new httpsserver::SSLCert(cert_crt,crt_len,cert_key,key_len);
  }
  _https_port = https_port > 0 ? https_port : CONF_WEB_HTTPS_PORT;
  log_d("initializing https_server on port %d", _https_port);
  _https_server = new httpsserver::HTTPSServer(https_cert,_https_port);
#endif
}

void WebPlatform::begin(const char* name, bool secure, void (*handle_notFound)(HTTPRequest*,HTTPResponse*)) {
  if (handle_notFound == NULL) {
    handle_notFound = [](HTTPRequest* req, HTTPResponse* res) {
      req->discardRequestBody();
      res->setStatusCode(404);
      res->setHeader("Content-Type", "text/plain");
      res->println("Not Found");
      res->finalize();
    };
  }
  httpsserver::ResourceNode* node404 = new httpsserver::ResourceNode("", HTTP_ANY, handle_notFound);
#ifdef CONF_WEB_HTTPS
  if (_https_server != NULL) {
    _https_server_redirect = secure;
  }
  if (_https_server_redirect) {
    httpsserver::ResourceNode* nodeHttpsRedirect = new httpsserver::ResourceNode("", HTTP_ANY, [](HTTPRequest* req, HTTPResponse* res) {
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
    Serial.print(WiFiUtils::getIP());
  }
  if (port != port_default) {
    Serial.print(":");
    Serial.print(port);
  }
  Serial.println("/");
}

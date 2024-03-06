#ifndef MODULO_WEB_H
#define MODULO_WEB_H

/**
 * Contiene variabili, tipi e funzioni per l'uso come Web Server.
 * 
 * @see https://github.com/fhessel/esp32_https_server/blob/master/examples/HTTPS-and-HTTP/HTTPS-and-HTTP.ino
 * @see https://github.com/espressif/arduino-esp32/blob/master/libraries/ESPmDNS/src/ESPmDNS.h
 */

#include "base_conf.h"
#include <HTTPServer.hpp>
#include <HTTPBodyParser.hpp>
#include <HTTPURLEncodedBodyParser.hpp>
#include <HTTPMultipartBodyParser.hpp>
#include <ESPmDNS.h>
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>

#define HTTP_ANY     ""
#define HTTP_GET     "GET"
#define HTTP_POST    "POST"

using namespace httpsserver;

uint16_t http_port = 0;
uint16_t https_port = 0;
HTTPServer * http_server = NULL;
HTTPSServer * https_server = NULL;
bool https_server_redirect = false;

void web_server_loop() {
  if (http_server != NULL) {
    http_server->loop();
  }
  if (https_server != NULL) {
    https_server->loop();
  }
}

bool web_request_post(HTTPRequest * req) {
  return req->getMethod().compare("POST") == 0;
}

bool web_authenticate(HTTPRequest * req, const char * username, const char * password) {
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

void web_authenticate_request(HTTPRequest * req, HTTPResponse * res) {
  log_d("authentication needed"); 
  req->discardRequestBody();
  res->setStatusCode(401);
  res->setStatusText("Unauthorized");
  res->setHeader("Content-Type", "text/plain");
  res->setHeader("WWW-Authenticate", "Basic realm=\"Privileged area\"");
  res->println("401. Unauthorized");
  res->finalize();
}

void web_cookie_create(HTTPRequest * req, HTTPResponse * res, const char * name, const char * value, unsigned long max_age = 0) {
  std::string cookieHeader = std::string(name) + "=" + value;
  if (max_age > 0) {
    cookieHeader += "; max-age=";
    cookieHeader += String(max_age).c_str();
  }
  res->setHeader("Set-Cookie", cookieHeader);
}

void web_cookie_delete(HTTPRequest * req, HTTPResponse * res, const char * name) {
  std::string cookieHeader = std::string(name) + "=; expires=Thu, 01 Jan 1970 00:00:00 GMT";
  res->setHeader("Set-Cookie", cookieHeader);
}

String web_cookie_read(HTTPRequest * req, HTTPResponse * res, const char * name) {
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

String web_parameter(HTTPBodyParser * parser) {
  std::string str = "";
  while (!parser->endOfField()) {
    char buf[513];
    size_t curr_len = parser->read((byte *)buf, sizeof(buf)-1);
    Serial.println(std::string(buf, curr_len).c_str());
    str = str + std::string(buf, curr_len);
  }
  return String(str.c_str());
}

String web_parameter(HTTPRequest * req, const char * param_name) {
  std::string str;
  auto params = req->getParams();
  if (params->getQueryParameter(param_name, str)) {
    return String(str.c_str());
  }
  return "";
}

void web_send_redirect(HTTPRequest * req, HTTPResponse * res, String redirect_uri, String message = "", int refresh = 0) {
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
  if (refresh < 0) {
    refresh = 5000;
  }
  res->setHeader("Connection", "close");
  res->printf("<html><body>%s</body><script>document.addEventListener('DOMContentLoaded',function(event){setTimeout(function(){location='%s'},%d)})</script></html>", message.c_str(), redirect_uri.c_str(), refresh);
}

void web_download_text(HTTPRequest * req, HTTPResponse * res, String content_type, String filename, String text) {
  req->discardRequestBody();
  res->setHeader("Content-Disposition", ("attachment;filename=\""+filename+"\"").c_str());
  res->setHeader("Content-Type", content_type.c_str());
  res->println(text);
  res->finalize();
}

void web_handle_notFound(HTTPRequest * req, HTTPResponse * res);

void web_server_register(ResourceNode * node) {
  if (http_server != NULL && !https_server_redirect) {
    http_server->registerNode(node);
  }
  if (https_server != NULL) {
    https_server->registerNode(node);
  }
}

void web_server_register(const std::string &method, const std::string &path, const HTTPSCallbackFunction * callback) {
  if (method == HTTP_ANY) {
    web_server_register(new ResourceNode(path, HTTP_GET, callback));
    web_server_register(new ResourceNode(path, HTTP_POST, callback));
  } else {
    web_server_register(new ResourceNode(path, method, callback));
  }
}

bool web_server_setup_http(const uint16_t port = CONF_WEB_HTTP_PORT) {
  http_port = port > 0 ? port : CONF_WEB_HTTP_PORT;
  log_d("initializing http_server on port %d", http_port);
  http_server = new HTTPServer(http_port);
  return true;
}

bool web_server_setup_https(String crt = "", String key = "", const uint16_t port = CONF_WEB_HTTPS_PORT) {
  if (crt == "" || key == "") {
    crt = CONF_WEB_HTTPS_CERT;
    key = CONF_WEB_HTTPS_CERT_KEY;
  }
  SSLCert * https_cert;
  if (crt == "" || key == "") {
    log_d("create self-signed cert");
    https_cert = new SSLCert();
    int res = createSelfSignedCert(*https_cert, KEYSIZE_1024, "CN=esp32.local,O=acme,C=IT");
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
    https_cert = new SSLCert(
      cert_crt, crt_len,
      cert_key, key_len
    );
  }
  https_port = port > 0 ? port : CONF_WEB_HTTPS_PORT;
  log_d("initializing https_server on port %d", https_port);
  https_server = new HTTPSServer(https_cert,https_port);
  return true;
}

void web_server_begin(String name,bool secure = false) {
  ResourceNode * node404 = new ResourceNode("", HTTP_GET, &web_handle_notFound);
  if (https_server != NULL) {
    https_server_redirect = secure;
  }
  if (https_server_redirect) {
    ResourceNode * nodeHttpsRedirect = new ResourceNode(CONF_WEB_URI_FIRMWARE_UPDATE, HTTP_GET, [](HTTPRequest * req, HTTPResponse * res) {
      std::string host = req->getHeader("Host");
      if (host == "") {
        host = CONF_WEB_HTTPS_NAME;
      }
      web_send_redirect(req,res,String(("https://" + host + req->getRequestString()).c_str()));
    });
    http_server->setDefaultNode(nodeHttpsRedirect);
  } else {
    http_server->setDefaultNode(node404);
  }
  http_server->start();
  String protocol;
  uint16_t port;
  uint16_t port_default;
  if (https_server == NULL) {
    protocol = "http";
    port = http_port;
    port_default = 80;
  } else {
    protocol = "https";
    port = https_port;
    port_default = 443;
    https_server->setDefaultNode(node404);
    https_server->start();
  }
  Serial.print("Ready on ");
  Serial.print(protocol);
  Serial.print("://");
  if (MDNS.begin(name.c_str())) {
    MDNS.addService(protocol, "tcp", port);
    Serial.print(name);
    Serial.print(".local");
  } else {
    Serial.print(WiFi.localIP());
  }
  if (port != port_default) {
    Serial.print(":");
    Serial.print(port);
  }
  Serial.println("/");
}

#endif

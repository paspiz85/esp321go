#ifndef INCLUDE_WEB_H
#define INCLUDE_WEB_H

/**
 * Contiene variabili, tipi e funzioni per l'uso come Web Server.
 * 
 * @see https://github.com/espressif/arduino-esp32/blob/master/libraries/WebServer/src/WebServer.h
 * @see https://github.com/espressif/arduino-esp32/blob/master/libraries/ESPmDNS/src/ESPmDNS.h
 */

#include "base_conf.h"
#include <WebServer.h>
#include <uri/UriBraces.h>
#include <uri/UriRegex.h>
#include <ESPmDNS.h>

uint16_t http_port = 0;
WebServer * web_server = NULL;

void web_server_loop() {
  if (web_server != NULL) {
   web_server->handleClient();
  }
}

bool web_request_post() {
  if (web_server == NULL) {
    return false;
  }
  return web_server->method() == HTTP_POST;
}

bool web_authenticate(const char * username, const char * password) {
  if (web_server == NULL) {
    return false;
  }
  return web_server->authenticate(username, password);
}

void web_authenticate_request() {
  if (web_server != NULL) {
    web_server->requestAuthentication();
  }
}

String web_parameter(String param_name) {
  if (web_server == NULL) {
    return "";
  }
  for (int i = 0; i < web_server->args(); i++) {
    if (web_server->argName(i) == param_name) {
      return web_server->arg(i);
    }
  }
  return "";
}

String web_path_arg(int n) {
  return web_server->pathArg(n);
}

String web_header(String name) {
  if (web_server == NULL) {
    return "";
  }
  return web_server->header(name);
}

HTTPUpload& web_upload() {
  return web_server->upload();
}

void web_send_redirect(String redirect_uri, String message = "", int refresh = 0) {
  if (web_server == NULL) {
    return;
  }
  if (redirect_uri == "") {
    redirect_uri = web_server->header("Referer");
    log_d("referer %s", redirect_uri.c_str());
  }
  if (redirect_uri == "") {
    redirect_uri = "/";
  }
  log_d("redirect_uri %s", redirect_uri.c_str());
  if (message == "") {
    web_server->sendHeader("Location", redirect_uri, true);
    web_server->sendHeader("Connection", "close");
    web_server->send(302,"text/plain","");
    return;
  }
  if (refresh < 0) {
    refresh = 5000;
  }
  web_server->sendHeader("Connection", "close");
  web_server->send(200, "text/html", "<html><body>"+message+"</body><script>document.addEventListener('DOMContentLoaded',function(event){setTimeout(function(){location='"+redirect_uri+"'},"+String(refresh)+")})</script></html>");
}

void web_send_text(int status_code, String content_type, String text) {
  if (web_server == NULL) {
    return;
  }
  web_server->send(status_code,content_type,text);
}

void web_download_text(String content_type, String filename, String text) {
  if (web_server == NULL) {
    return;
  }
  web_server->sendHeader("Content-Disposition", "attachment;filename=\""+filename+"\"");
  web_server->send(200,content_type,text);
}

void web_handle_notFound();

void web_server_register(HTTPMethod method, const Uri &uri, WebServer::THandlerFunction fn) {
  if (web_server == NULL) {
    return;
  }
  web_server->on(uri, method, fn);
}

void web_server_register(HTTPMethod method, const Uri &uri, WebServer::THandlerFunction fn, WebServer::THandlerFunction ufn) {
  if (web_server == NULL) {
    return;
  }
  web_server->on(uri, method, fn, ufn);
}

void web_server_setup_http(const uint16_t port = CONF_WEB_HTTP_PORT) {
  http_port = port > 0 ? port : CONF_WEB_HTTP_PORT;
  web_server = new WebServer(port);
}

void web_server_begin(String name) {
  if (web_server == NULL) {
    return;
  }
  web_server->onNotFound(web_handle_notFound);
  const char * headerkeys[] = {"Accept", "Referer"};
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  web_server->collectHeaders(headerkeys, headerkeyssize);
  web_server->begin();
  Serial.print("Ready on http://");
  if (MDNS.begin(name.c_str())) {
    MDNS.addService("http", "tcp", http_port);
    Serial.print(name);
    Serial.print(".local");
  } else {
    Serial.print(WiFi.localIP());
  }
  if (http_port != 80) {
    Serial.print(":");
    Serial.print(http_port);
  }
  Serial.println("/");
}

#endif

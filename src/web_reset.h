#pragma once

/**
 * Contiene variabili, tipi e funzioni per il reset del Web Server.
 */

#include "base_conf.h"
#include "web_admin_component.h"

void web_reset(HTTPRequest* req, HTTPResponse* res, const String& message, int refresh = 0) {
  log_i("reset");
  web_sendRedirect(req,res,"/",message,refresh);
  delay(1000);
  ESP.restart();
}

class WebReset {
private:
  String _web_uri;
public:
  WebReset(WebPlatform* platform, const String& uri) {
    _web_uri = uri;
    platform->handle(HTTP_ANY, _web_uri, [](HTTPRequest* req, HTTPResponse* res) {
      if (web_authenticateAdmin(req,res)) {
        return;
      }
      if (!web_isRequestMethodPost(req)) {
        return web_sendRedirect(req,res,"/");
      }
      web_reset(req,res,"OK");
    });
  };
  const String& getUri() {
    return _web_uri;
  };
};

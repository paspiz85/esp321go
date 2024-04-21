#ifndef INCLUDE_WEB_RESET_H
#define INCLUDE_WEB_RESET_H

/**
 * Contiene variabili, tipi e funzioni per il reset del Web Server.
 */

#include "base_conf.h"
#include "web.h"

bool (*__web_reset_admin_authenticate)(HTTPRequest * req) = nullptr;

void web_reset(HTTPRequest * req, HTTPResponse * res, String message, int refresh = 0) {
  log_i("reset");
  Web.sendRedirect(req,res,"/",message,refresh);
  delay(1000);
  ESP.restart();
}

void web_reset_setup(bool (*web_admin_authenticate)(HTTPRequest * req) = nullptr) {
  __web_reset_admin_authenticate = web_admin_authenticate;
  Web.handle(HTTP_ANY, CONF_WEB_URI_RESET, [](HTTPRequest * req, HTTPResponse * res) {
    if (__web_reset_admin_authenticate != NULL) {
      if (!__web_reset_admin_authenticate(req)) {
        return Web.authenticateRequest(req,res);
      }
    }
    if (!Web.isRequestMethodPost(req)) {
      return Web.sendRedirect(req,res,"/");
    }
    web_reset(req,res,"OK");
  });
}

#endif

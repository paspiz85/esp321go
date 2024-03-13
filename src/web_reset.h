#ifndef INCLUDE_WEB_RESET_H
#define INCLUDE_WEB_RESET_H

/**
 * Contiene variabili, tipi e funzioni per il reset del Web Server.
 */

#include "base_conf.h"
#include "web.h"
#include "web_admin.h"

void web_reset(HTTPRequest * req, HTTPResponse * res, String message, int refresh = 0) {
  log_i("reset");
  web_send_redirect(req,res,"/",message,refresh);
  delay(1000);
  ESP.restart();
}

void web_reset_setup() {
  web_server_register(HTTP_ANY, CONF_WEB_URI_RESET, [](HTTPRequest * req, HTTPResponse * res) {
    if (!web_admin_authenticate(req)) {
      return web_authenticate_request(req,res);
    }
    if (!web_request_post(req)) {
      return web_send_redirect(req,res,"/");
    }
    web_reset(req,res,"OK");
  });
}

#endif

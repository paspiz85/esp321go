#ifndef MODULO_WEB_RESET_H
#define MODULO_WEB_RESET_H

/**
 * Contiene variabili, tipi e funzioni per il reset del Web Server.
 */

#include "web.h"
#include "web_admin.h"

void web_reset(String message, int refresh = 0) {
  log_i("reset");
  web_send_redirect("/",message,refresh);
  delay(1000);
  ESP.restart();
}

void web_reset_setup() {
  web_server_register(HTTP_ANY, CONF_WEB_URI_RESET, []() {
    if (!web_admin_authenticate()) {
      return web_authenticate_request();
    }
    if (!web_request_post()) {
      return web_send_redirect("/");
    }
    web_reset("OK");
  });
}

#endif

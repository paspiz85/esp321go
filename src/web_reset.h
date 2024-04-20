#ifndef INCLUDE_WEB_RESET_H
#define INCLUDE_WEB_RESET_H

/**
 * Contiene variabili, tipi e funzioni per il reset del Web Server.
 */

#include "base_conf.h"
#include "web.h"

bool (*web_reset_admin_authenticate)() = nullptr;

void web_reset(String message, int refresh = 0) {
  log_i("reset");
  Web.sendRedirect("/",message,refresh);
  delay(1000);
  ESP.restart();
}

void web_reset_setup(bool (*web_admin_authenticate)() = nullptr) {
  web_reset_admin_authenticate = web_admin_authenticate;
  Web.handle(HTTP_ANY, CONF_WEB_URI_RESET, []() {
    if (web_reset_admin_authenticate != NULL) {
      if (!web_reset_admin_authenticate()) {
        return Web.authenticateRequest();
      }
    }
    if (!Web.isRequestMethodPost()) {
      return Web.sendRedirect("/");
    }
    web_reset("OK");
  });
}

#endif

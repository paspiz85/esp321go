#ifndef MODULO_WEB_ADMIN_H
#define MODULO_WEB_ADMIN_H

/**
 * Contiene l'autenticazione dell'amministratore del Web Server.
 */

#include "web.h"

String web_admin_username;
String web_admin_password;

bool web_admin_authenticate(HTTPRequest * req) {
  return web_authenticate(req, web_admin_username.c_str(), web_admin_password.c_str());
}

void web_admin_setup() {
  web_admin_username = preferences.getString(PREF_ADMIN_USERNAME,CONF_ADMIN_USERNAME);
  web_admin_password = preferences.getString(PREF_ADMIN_PASSWORD,CONF_ADMIN_PASSWORD);
}

#endif

#ifndef INCLUDE_WEB_ADMIN_H
#define INCLUDE_WEB_ADMIN_H

/**
 * Contiene l'autenticazione dell'amministratore del Web Server.
 */

#include "web.h"

String web_admin_username;
String web_admin_password;

bool web_admin_authenticate() {
  return Web.authenticate(web_admin_username.c_str(), web_admin_password.c_str());
}

void web_admin_setup(const char * username, const char * password) {
  web_admin_username = String(username);
  web_admin_password = String(password);
}

#endif

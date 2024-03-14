#ifndef INCLUDE_WEB_ADMIN_H
#define INCLUDE_WEB_ADMIN_H

/**
 * Contiene l'autenticazione dell'amministratore del Web Server.
 */

#include "web.h"

const char * web_admin_username;
const char * web_admin_password;

bool web_admin_authenticate() {
  return web_authenticate(web_admin_username, web_admin_password);
}

void web_admin_setup(const char * username, const char * password) {
  web_admin_username = username;
  web_admin_password = password;
}

#endif

#ifndef INCLUDE_WEB_USERS_H
#define INCLUDE_WEB_USERS_H

/**
 * Contiene l'autenticazione degli utenti.
 */

#include "base_utils.h"
#include "web_platform.h"
#include <Arduino_JSON.h>

#define USER_COOKIE "esp32auth"
#define USER_COOKIE_MAXAGE (23328000)
// 9 mesi
#define USER_PASSWORD "password"

class WebUsers {
private:
  WebPlatform* _platform;
  JSONVar _users;
public:
  WebUsers(WebPlatform* platform, const String& admin_username, const String& admin_password, String config);
  bool login();
  void logout();
};

bool WebUsers::login() {
  String cookie = _platform->cookieRead(USER_COOKIE);
  if (cookie != "") {
    String value = base64_decode_str(cookie);
    int i = value.indexOf(':');
    if (i != -1) {
      String username = value.substring(0, i);
      String password = value.substring(i + 1);
      log_d("username %s", username.c_str());
      log_d("password %s", password.c_str());
      if (_users.hasOwnProperty(username)) {
        JSONVar user = _users[username];
        String user_password = user[USER_PASSWORD];
        if (password.equals(user_password)) {
          return true;
        }
      }
    }
  }
  _platform->sendRedirect(CONF_WEB_URI_LOGIN);
  return false;
}

void WebUsers::logout() {
  _platform->cookieDelete(USER_COOKIE);
  _platform->cacheControl(false);
  _platform->sendRedirect("/");
}

WebUsers::WebUsers(WebPlatform* platform, const String& admin_username, const String& admin_password, String config) {
  _platform = platform;
  if (config == "") {
    config = "{}";
  }
  log_d("users config is %s", config);
  _users = JSON.parse(config);
  JSONVar admin;
  admin[USER_PASSWORD] = admin_password;
  _users[admin_username] = admin;
  log_d("users are %s", JSON.stringify(_users));
  _platform->handle(HTTP_ANY, CONF_WEB_URI_LOGOUT, [this]() {
    this->logout();
  });
  _platform->handle(HTTP_ANY, CONF_WEB_URI_LOGIN, [this]() {
    JSONVar keys = this->_users.keys();
    for (int i = 0; i < keys.length(); i++) {
      const char * key = keys[i];
      JSONVar user = this->_users[key];
      String user_password = user[USER_PASSWORD];
      if (this->_platform->authenticate(key, user_password.c_str())) {
        this->_platform->cookieCreate(USER_COOKIE,base64_encode_str(String(key)+":"+user_password).c_str(),USER_COOKIE_MAXAGE);
        this->_platform->cacheControl(false);
        return this->_platform->sendRedirect("/");
      }
    }
    return this->_platform->authenticateRequest();
  });
}

#endif
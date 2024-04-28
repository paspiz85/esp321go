#pragma once

/**
 * Contiene l'autenticazione degli utenti.
 */

#include "base_utils.h"
#include "web_platform.h"
#include <Arduino_JSON.h>

namespace WebUsers {

#define USER_COOKIE "esp32auth"
#define USER_COOKIE_MAXAGE (23328000)
// 9 mesi
#define USER_PASSWORD "password"

JSONVar _users;

bool login(HTTPRequest* req, HTTPResponse* res) {
  String cookie = web_cookieRead(req,USER_COOKIE);
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
  web_sendRedirect(req,res,CONF_WEB_URI_LOGIN);
  return false;
}

void logout(HTTPRequest* req, HTTPResponse* res) {
  web_cookieDelete(req,res,USER_COOKIE);
  web_cacheControl(req,res,false);
  web_sendRedirect(req,res,"/");
}

void setup(WebPlatform* platform, const String& admin_username, const String& admin_password, const String& config) {
  log_d("users config is %s", config.c_str());
  _users = JSON.parse(config == "" ? "{}" : config);
  JSONVar admin;
  admin[USER_PASSWORD] = admin_password;
  _users[admin_username] = admin;
  log_d("users are %s", JSON.stringify(_users).c_str());
  platform->handle(HTTP_ANY, CONF_WEB_URI_LOGOUT, [](HTTPRequest* req, HTTPResponse* res) {
    logout(req,res);
  });
  platform->handle(HTTP_ANY, CONF_WEB_URI_LOGIN, [](HTTPRequest* req, HTTPResponse* res) {
    JSONVar keys = _users.keys();
    for (int i = 0; i < keys.length(); i++) {
      const char* key = keys[i];
      JSONVar user = _users[key];
      String user_password = user[USER_PASSWORD];
      if (web_authenticate(req,key,user_password.c_str())) {
        web_cookieCreate(req,res,USER_COOKIE,base64_encode_str(String(key)+":"+user_password),USER_COOKIE_MAXAGE);
        web_cacheControl(req,res,false);
        return web_sendRedirect(req,res,"/");
      }
    }
    web_authenticateRequest(req,res);
    return;
  });
}

}

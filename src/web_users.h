#ifndef INCLUDE_WEB_USERS_H
#define INCLUDE_WEB_USERS_H

/**
 * Contiene l'autenticazione degli utenti.
 */

#include "config.h"
#include "web.h"
#include "web_templates.h"
#include <Arduino_JSON.h>

#define USER_PASSWORD "password"

JSONVar web_users;

bool web_login(HTTPRequest * req, HTTPResponse * res) {
  String cookie = Web.cookieRead(req,res,"esp32auth");
  if (cookie != "") {
    String value = base64_decode_str(cookie);
    int i = value.indexOf(':');
    if (i != -1) {
      String username = value.substring(0, i);
      String password = value.substring(i + 1);
      log_d("username %s", username.c_str());
      log_d("password %s", password.c_str());
      if (web_users.hasOwnProperty(username)) {
        JSONVar user = web_users[username];
        String user_password = user[USER_PASSWORD];
        if (password.equals(user_password)) {
          return true;
        }
      }
    }
  }
  Web.sendRedirect(req,res,CONF_WEB_URI_LOGIN);
  return false;
}

void web_logout(HTTPRequest * req, HTTPResponse * res) {
  Web.cookieDelete(req,res,"esp32auth");
  Web.cacheControl(req,res,false);
  return Web.sendRedirect(req,res,"/");
}

void web_users_setup(String admin_username, String admin_password) {
  String config = preferences.getString(PREF_WEB_USERS);
  if (config == "") {
    config = "{}";
  }
  log_d("users config is %s", config);
  web_users = JSON.parse(config);
  JSONVar admin;
  admin[USER_PASSWORD] = admin_password;
  web_users[admin_username] = admin;
  log_d("users are %s", JSON.stringify(web_users));
  Web.handle(HTTP_ANY, CONF_WEB_URI_LOGOUT, &web_logout);
  Web.handle(HTTP_ANY, CONF_WEB_URI_LOGIN, [](HTTPRequest * req, HTTPResponse * res) {
    JSONVar keys = web_users.keys();
    for (int i = 0; i < keys.length(); i++) {
      const char * key = keys[i];
      JSONVar user = web_users[key];
      String user_password = user[USER_PASSWORD];
      if (Web.authenticate(req, key, user_password.c_str())) {
        Web.cookieCreate(req,res,"esp32auth",base64_encode_str(String(key)+":"+user_password).c_str(),23328000); // 9 mesi
        Web.cacheControl(req,res,false);
        return Web.sendRedirect(req,res,"/");
      }
    }
    return Web.authenticateRequest(req,res);
  });
}

#endif

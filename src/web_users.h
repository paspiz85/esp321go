#ifndef MODULO_WEB_USERS_H
#define MODULO_WEB_USERS_H

/**
 * Contiene l'autenticazione degli utenti.
 */

#include "config.h"
#include "web.h"
#include "web_admin.h"
#include "web_pages.h"
#include <Arduino_JSON.h>

#define USER_PASSWORD "password"

JSONVar web_users;

bool web_login(HTTPRequest * req, HTTPResponse * res) {
  String cookie = web_cookie_read(req,res,"esp32auth");
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
  web_send_redirect(req,res,CONF_WEB_URI_LOGIN);
  return false;
}

void web_logout(HTTPRequest * req, HTTPResponse * res) {
  web_cookie_delete(req,res,"esp32auth");
  web_cache_control(req,res,false);
  return web_send_redirect(req,res,"/");
}

void web_users_setup() {
  web_admin_setup();
  String config = preferences.getString(PREF_USERS);
  if (config == "") {
    config = "{}";
  }
  log_d("users config is %s", config);
  web_users = JSON.parse(config);
  JSONVar admin;
  admin[USER_PASSWORD] = web_admin_password;
  web_users[web_admin_username] = admin;
  log_d("users are %s", JSON.stringify(web_users));
  web_server_register(HTTP_ANY, CONF_WEB_URI_LOGOUT, &web_logout);
  web_server_register(HTTP_ANY, CONF_WEB_URI_LOGIN, [](HTTPRequest * req, HTTPResponse * res) {
    JSONVar keys = web_users.keys();
    for (int i = 0; i < keys.length(); i++) {
      const char * key = keys[i];
      JSONVar user = web_users[key];
      String user_password = user[USER_PASSWORD];
      if (web_authenticate(req, key, user_password.c_str())) {
        web_cookie_create(req,res,"esp32auth",base64_encode_str(String(key)+":"+user_password).c_str(),23328000); // 9 mesi
        web_cache_control(req,res,false);
        return web_send_redirect(req,res,"/");
      }
    }
    return web_authenticate_request(req,res);
  });
}

#endif

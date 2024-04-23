#ifndef INCLUDE_WEB_RESET_H
#define INCLUDE_WEB_RESET_H

/**
 * Contiene variabili, tipi e funzioni per il reset del Web Server.
 */

#include "base_conf.h"
#include "web_admin_component.h"

void web_reset(WebPlatform* platform, String message, int refresh = 0) {
  log_i("reset");
  platform->sendRedirect("/",message,refresh);
  delay(1000);
  ESP.restart();
}

class WebReset : public WebAdminComponent {
private:
  String _web_uri;
public:
  WebReset(WebPlatform* platform, const String& uri, bool (*web_admin_authenticate)() = nullptr) : WebAdminComponent(platform,web_admin_authenticate) {
    _web_uri = uri;
    platform->handle(HTTP_ANY, _web_uri, [this]() {
      if (this->authenticateAdmin()) {
        return;
      }
      if (!this->getPlatform()->isRequestMethodPost()) {
        return this->getPlatform()->sendRedirect("/");
      }
      web_reset(this->getPlatform(),"OK");
    });
  };
  const String& getUri() {
    return _web_uri;
  };
};

WebPlatform* __web_reset_platform = nullptr;
bool (*__web_reset_admin_authenticate)() = nullptr;

void web_reset_setup(WebPlatform* platform, bool (*web_admin_authenticate)() = nullptr) {
  __web_reset_platform = platform;
  __web_reset_admin_authenticate = web_admin_authenticate;
  __web_reset_platform->handle(HTTP_ANY, CONF_WEB_URI_RESET, []() {
    if (__web_reset_admin_authenticate != NULL) {
      if (!__web_reset_admin_authenticate()) {
        return __web_reset_platform->authenticateRequest();
      }
    }
    if (!__web_reset_platform->isRequestMethodPost()) {
      return __web_reset_platform->sendRedirect("/");
    }
    web_reset(__web_reset_platform,"OK");
  });
}

#endif

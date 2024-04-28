#pragma once

/**
 * Contiene variabili, tipi e funzioni per il reset del Web Server.
 */

#include "base_conf.h"
#include "web_admin_component.h"

void web_reset(WebPlatform* platform, const String& message, int refresh = 0) {
  log_i("reset");
  platform->sendRedirect("/",message,refresh);
  delay(1000);
  ESP.restart();
}

class WebReset : public WebAdminComponent {
private:
  String _web_uri;
public:
  WebReset(WebPlatform* platform, const String& uri, std::function<bool(void)> web_admin_authenticate = NULL) : WebAdminComponent(platform,web_admin_authenticate) {
    _web_uri = uri;
    platform->handle(HTTP_ANY, _web_uri, [this]() {
      if (authenticateAdmin()) {
        return;
      }
      if (!getPlatform()->isRequestMethodPost()) {
        return getPlatform()->sendRedirect("/");
      }
      web_reset(getPlatform(),"OK");
    });
  };
  const String& getUri() {
    return _web_uri;
  };
};

#pragma once

#include "web_platform.h"

class WebAdminComponent {
private:
  WebPlatform* _platform;
  std::function<bool(void)> _web_admin_authenticate;
public:
  WebAdminComponent(WebPlatform* platform, std::function<bool(void)> web_admin_authenticate = NULL) {
    _platform = platform;
    _web_admin_authenticate = web_admin_authenticate;
  };
  WebPlatform* getPlatform() {
    return _platform;
  };
  bool authenticateAdmin() {
    if (_web_admin_authenticate != NULL) {
      if (!_web_admin_authenticate()) {
        _platform->authenticateRequest();
        return true;
      }
    }
    return false;
  };
};

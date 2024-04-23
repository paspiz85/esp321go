#ifndef INCLUDE_WEB_ADMIN_COMPONENT_H
#define INCLUDE_WEB_ADMIN_COMPONENT_H

#include "web_platform.h"

class WebAdminComponent {
private:
  WebPlatform* _platform;
  bool (*_web_admin_authenticate)();
public:
  WebAdminComponent(WebPlatform* platform, bool (*web_admin_authenticate)() = nullptr) {
    _platform = platform;
    _web_admin_authenticate = web_admin_authenticate;
  };
  WebPlatform* getPlatform() {
    return _platform;
  };
  virtual bool authenticateAdmin() {
    if (_web_admin_authenticate != NULL) {
      if (!_web_admin_authenticate()) {
        _platform->authenticateRequest();
        return true;
      }
    }
    return false;
  };
};

#endif

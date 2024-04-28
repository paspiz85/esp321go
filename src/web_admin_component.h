#pragma once

#include "web_platform.h"

bool web_admin_authenticate(HTTPRequest* req);

bool web_authenticateAdmin(HTTPRequest* req, HTTPResponse* res) {
  if (!web_admin_authenticate(req)) {
    web_authenticateRequest(req,res);
    return true;
  }
  return false;
};

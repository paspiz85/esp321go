#pragma once

/**
 * Contiene funzioni per la gestione delle pagine HTML del Web Server.
 */

#include "base_utils.h"
#include "web_platform.h"

const char* __web_templates_css =
"body{font-family:system-ui;font-size:1rem;font-weight:400;line-height:1.5}"
"fieldset{min-width:0;padding:0;margin:0;border:0}"
"form fieldset>div{margin-top:.25rem;margin-bottom:.5rem}"
"table,td,th{border:1px solid #000;border-collapse:collapse}"
"th,td{padding:5px}"
"pre{text-align:start;font-size:initial}"
".btn{display:inline-block;line-height:1.5;text-align:center;text-decoration:none;vertical-align:middle;cursor:pointer;-webkit-user-select:none;-moz-user-select:none;user-select:none;background-color:transparent;border:1px solid transparent;padding:.375rem .75rem;font-size:1rem;border-radius:.25rem;transition:color .15s ease-in-out,background-color .15s ease-in-out,border-color .15s ease-in-out,box-shadow .15s ease-in-out}"
".btn-primary{color:#fff;background-color:#0a58ca;border-color:#0a58ca}"
".btn-secondary{color:#fff;background-color:#5c636a;border-color:#5c636a}"
".btn-danger{color:#fff;background-color:#dc3545;border-color:#dc3545}"
".btn-success{color:#fff;background-color:#198754;border-color:#198754}"
".d-none{display:none!important}"
".form-control{display:block;width:100%;padding:.375rem .75rem;font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#fff;background-clip:padding-box;border:1px solid #ced4da;-webkit-appearance:none;-moz-appearance:none;appearance:none;border-radius:.25rem;transition:border-color .15s ease-in-out,box-shadow .15s ease-in-out}"
".form-select{display:block;width:100%;padding:.375rem 2.25rem .375rem .75rem;-moz-padding-start:calc(0.75rem - 3px);font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#fff;background-image:url(data:image/svg+xml,%3csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 16 16'%3e%3cpath fill='none' stroke='%23343a40' stroke-linecap='round' stroke-linejoin='round' stroke-width='2' d='M2 5l6 6 6-6'/%3e%3c/svg%3e);background-repeat:no-repeat;background-position:right .75rem center;background-size:16px 12px;border:1px solid #ced4da;border-radius:.25rem;transition:border-color .15s ease-in-out,box-shadow .15s ease-in-out;-webkit-appearance:none;-moz-appearance:none;appearance:none}"
".fs-03{font-size:calc(3rem + 6vw)!important}"
".fs-1{font-size:calc(1.375rem + 1.5vw)!important}"
".fw-bold{font-weight:700!important}"
".input-group{position:relative;display:flex;flex-wrap:wrap;align-items:stretch;width:100%}"
".input-group>:not(:first-child){margin-left:-1px;border-top-left-radius:0;border-bottom-left-radius:0}"
".input-group>:not(:last-child){border-top-right-radius:0;border-bottom-right-radius:0}"
".input-group>.form-control,.input-group>.form-select{position:relative;flex:1 1 auto;width:1%;min-width:0}"
".input-group-text{display:flex;align-items:center;padding:.375rem .75rem;font-size:1rem;font-weight:400;line-height:1.5;color:#212529;text-align:center;white-space:nowrap;background-color:#e9ecef;border:1px solid #ced4da;border-radius:.25rem}"
".text-center{text-align:center}";

class WebGUI : public WebPlatform {
private:
  String _title;
  String (*_footer)(bool) = NULL;
public:
#ifdef CONF_WEB_HTTPS
  WebGUI(const uint16_t http_port = CONF_WEB_HTTP_PORT, const uint16_t https_port = CONF_WEB_HTTPS_PORT, String crt = "", String key = "") : WebPlatform(http_port,https_port,crt,key) {};
#else
  WebGUI(const uint16_t http_port = CONF_WEB_HTTP_PORT) : WebPlatform(http_port) {};
#endif
  void begin(const char* name = "", bool secure = false, void (*handle_notFound)(HTTPRequest*,HTTPResponse*) = NULL) override {
    handle(HTTP_ANY, "/css/style.css", [](HTTPRequest* req, HTTPResponse* res) {
      web_sendResponse(req,res,200,"text/css",__web_templates_css);
    });
    WebPlatform::begin(name,secure,handle_notFound);
  };
  void setTitle(const String& title) {
    _title = title;
  };
  void setFooter(String (*footer)(bool)) {
    _footer = footer;
  };
  String getTitle() {
    return _title;
  };
  String getFooter(bool admin = false) {
    if (_footer == NULL) {
      return "";
    }
    return _footer(admin);
  };
  void sendErrorClient(HTTPRequest* req, HTTPResponse* res, const String& message);
  void sendPage(HTTPRequest* req, HTTPResponse* res, const String& title, const String& body, uint16_t refresh = 0);
};

void WebGUI::sendErrorClient(HTTPRequest* req, HTTPResponse* res, const String& message) {
  log_d("client_error %s", message);
  req->discardRequestBody();
  res->setStatusCode(400);
  res->setHeader("Content-Type", "text/plain");
  res->println(message);
  res->finalize();
}

void WebGUI::sendPage(HTTPRequest* req, HTTPResponse* res, const String& title, const String& body, uint16_t refresh) {
  req->discardRequestBody();
  res->setHeader("Content-Type", "text/html");
  res->println("<!DOCTYPE html>");
  res->println("<html><meta charset=\"utf-8\">");
  res->println("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">");
  res->println("<meta http-equiv=\"Cache-Control\" content=\"no-cache\">");
  res->println("<meta http-equiv=\"Pragma\" content=\"no-cache\">");
  res->println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  if (refresh > 0) {
    res->printf("<meta http-equiv=\"refresh\" content=\"%d\">",refresh);
  }
  if (title != "") {
    res->printf("<title>%s</title>", html_encode(title));
  }
  res->println("<link rel=\"stylesheet\" type=\"text/css\" href=\"/css/style.css\">");
  res->println("</head>");
  res->println(body);
  res->println("</html>");
  res->finalize();
}

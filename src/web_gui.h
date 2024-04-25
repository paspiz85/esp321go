#ifndef INCLUDE_WEB_GUI_H
#define INCLUDE_WEB_GUI_H

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
  void begin(const char* name = "", std::function<void(void)> handle_notFound = NULL) override {
    handle(HTTP_ANY, "/css/style.css", [this]() {
      sendResponse(200, "text/css", __web_templates_css);
    });
    WebPlatform::begin(name,handle_notFound);
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
  void sendPage(const String& title, const String& body, uint16_t refresh = 0);
};

void WebGUI::sendPage(const String& title, const String& body, uint16_t refresh) {
  String html = "<!DOCTYPE html><html><head><meta charset=\"utf-8\">";
  html += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">";
  html += "<meta http-equiv=\"Cache-Control\" content=\"no-cache\">";
  html += "<meta http-equiv=\"Pragma\" content=\"no-cache\">";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  if (refresh > 0) {
    html += "<meta http-equiv=\"refresh\" content=\""+String(refresh)+"\">";
  }
  if (title != "") {
    html += "<title>"+html_encode(title)+"</title>";
  }
  html += "<link rel=\"stylesheet\" type=\"text/css\" href=\"/css/style.css\" />";
  html += "</head>";
  const char* chunks[] = {html.c_str(), body.c_str(), "</html>"};
  sendResponse(200, "text/html", len_array(chunks), chunks);
}

#endif

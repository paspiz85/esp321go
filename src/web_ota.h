#pragma once

/**
 * Contiene variabili, tipi e funzioni per l'aggiornamento tramite Web Server.
 */

#include "base_conf.h"
#include "web_gui.h"
#include "web_reset.h"
#ifdef PLATFORM_ESP32
#include <Update.h>
#endif

#ifndef UPDATE_SIZE_UNKNOWN
#define UPDATE_SIZE_UNKNOWN 0x00100000
#endif

class WebOTA : public WebAdminComponent {
private:
  WebGUI* _web_gui;
  String _web_uri;
public:
  WebOTA(WebGUI* web_gui, const String& uri, std::function<bool(void)> web_admin_authenticate = NULL) : WebAdminComponent(web_gui,web_admin_authenticate) {
    _web_gui = web_gui;
    _web_uri = uri;
    _web_gui->handle(HTTP_GET, _web_uri, [this]() {
      if (authenticateAdmin()) {
        return;
      }
      String title = _web_gui->getTitle();
      String html = "<body style=\"text-align:center\"><h1>"+title+"</h1><h2>Firmware Update</h2>";
  #ifdef PLATFORM_ESP32
      html += "<p>ESP32 Chip model "+String(ESP.getChipModel())+" - Rev "+String(ESP.getChipRevision())+"</p>";
      html += "<p>Chip ID "+String(ESP_getChipId())+" - Core# "+String(ESP.getChipCores())+"</p>";
  #else
      html += "<p>ESP8266 Chip ID "+String(ESP_getChipId())+"</p>";
  #endif
      html += "<form action=\""+_web_uri+"\" method=\"POST\" enctype=\"multipart/form-data\"><p>";
      html += "<input type=\"file\" name=\"update\" accept=\".bin\" required=\"required\" />";
      html += "</p><p>";
      html += "<button type=\"submit\" class=\"btn btn-primary\">Update</button> ";
      html += "<button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='/'\">Cancel</button>";
      html += "</p></form>";
      html += _web_gui->getFooter();
      html += "</body>";
      _web_gui->sendPage(title,html);
    });
    _web_gui->handleUpload(HTTP_POST, _web_uri, [this]() {
      web_reset(getPlatform(), Update.hasError() ? "FAILED" : "COMPLETED");
    }, [this]() {
      if (authenticateAdmin()) {
        return;
      }
      HTTPUpload& upload = _web_gui->getUpload();
      if (upload.status == UPLOAD_FILE_START) {
        log_i("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        /* flashing firmware to ESP*/
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          log_i("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
      }
    });
  };
  const String& getUri() {
    return _web_uri;
  };
};

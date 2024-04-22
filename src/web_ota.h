#ifndef INCLUDE_WEB_OTA_H
#define INCLUDE_WEB_OTA_H

/**
 * Contiene variabili, tipi e funzioni per l'aggiornamento tramite Web Server.
 */

#include "base_conf.h"
#include "web.h"
#include "web_templates.h"
#include "web_reset.h"
#ifdef PLATFORM_ESP32
#include <Update.h>
#endif

#ifndef UPDATE_SIZE_UNKNOWN
#define UPDATE_SIZE_UNKNOWN 0x00100000
#endif

bool (*__web_ota_admin_authenticate)() = nullptr;

void web_ota_setup(bool (*web_admin_authenticate)() = nullptr) {
  __web_ota_admin_authenticate = web_admin_authenticate;
  Web.handle(HTTP_GET, CONF_WEB_URI_FIRMWARE_UPDATE, []() {
    if (__web_ota_admin_authenticate != NULL) {
      if (!__web_ota_admin_authenticate()) {
        return Web.authenticateRequest();
      }
    }
    String title = WebTemplates.getTitle();
    String html = "<body style=\"text-align:center\"><h1>"+title+"</h1><h2>Firmware Update</h2>";
#ifdef PLATFORM_ESP32
    html += "<p>ESP32 Chip model "+String(ESP.getChipModel())+" - Rev "+String(ESP.getChipRevision())+"</p>";
    html += "<p>Chip ID "+String(ESP_getChipId())+" - Core# "+String(ESP.getChipCores())+"</p>";
#else
    html += "<p>ESP8266 Chip ID "+String(ESP_getChipId())+"</p>";
#endif
    html += "<form action=\""+String(CONF_WEB_URI_FIRMWARE_UPDATE)+"\" method=\"POST\" enctype=\"multipart/form-data\"><p>";
    html += "<input type=\"file\" name=\"update\" accept=\".bin\" required=\"required\" />";
    html += "</p><p>";
    html += "<button type=\"submit\" class=\"btn btn-primary\">Update</button> ";
    html += "<button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='/'\">Cancel</button>";
    html += "</p></form>";
    html += "</body>";
    WebTemplates.sendPage(title,html);
  });
  Web.handleUpload(HTTP_POST, CONF_WEB_URI_FIRMWARE_UPDATE, []() {
    web_reset(Update.hasError() ? "FAILED" : "COMPLETED");
  }, []() {
    if (__web_ota_admin_authenticate != NULL) {
      if (!__web_ota_admin_authenticate()) {
        return Web.authenticateRequest();
      }
    }
    HTTPUpload& upload = Web.getUpload();
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
}
#endif

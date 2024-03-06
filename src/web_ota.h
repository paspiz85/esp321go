#ifndef MODULO_WEB_OTA_H
#define MODULO_WEB_OTA_H

/**
 * Contiene variabili, tipi e funzioni per l'aggiornamento tramite Web Server.
 */

#include "base_conf.h"
#include "web.h"
#include "web_admin.h"
#include "web_pages.h"
#include "web_reset.h"
#include <Update.h>

uint32_t web_ota_chip_id() {
  uint32_t chipId = 0;
  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  return chipId;
}

void web_ota_setup() {
  web_server_register(HTTP_GET, CONF_WEB_URI_FIRMWARE_UPDATE, []() {
    if (!web_admin_authenticate()) {
      return web_authenticate_request();
    }
    String title = web_html_title();
    String html = "<body style=\"text-align:center\"><h1>"+title+"</h1><h2>Firmware Update</h2>";
    html += "<p>ESP32 Chip model "+String(ESP.getChipModel())+" - Rev "+String(ESP.getChipRevision())+"</p>";
    html += "<p>Chip ID "+String(web_ota_chip_id())+" - Core# "+String(ESP.getChipCores())+"</p>";
    html += "<form action=\""+String(CONF_WEB_URI_FIRMWARE_UPDATE)+"\" method=\"POST\" enctype=\"multipart/form-data\"><p>";
    html += "<input type=\"file\" name=\"update\" accept=\".bin\" required=\"required\" />";
    html += "</p><p>";
    html += "<button type=\"submit\" class=\"btn btn-primary\">Update</button> ";
    html += "<button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='/'\">Cancel</button>";
    html += "</p></form>";
    html += "</body>";
    web_send_page(title,html);
  });
  web_server_register(HTTP_POST, CONF_WEB_URI_FIRMWARE_UPDATE, []() {
    web_reset(Update.hasError() ? "FAILED" : "COMPLETED");
  }, []() {
    if (!web_admin_authenticate()) {
      return web_authenticate_request();
    }
    HTTPUpload& upload = web_server.upload();
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

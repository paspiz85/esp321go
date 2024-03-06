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
  web_server_register(HTTP_GET, CONF_WEB_URI_FIRMWARE_UPDATE, [](HTTPRequest * req, HTTPResponse * res) {
    if (!web_admin_authenticate(req)) {
      return web_authenticate_request(req,res);
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
    web_send_page(req,res,title,html);
  });
  web_server_register(HTTP_POST, CONF_WEB_URI_FIRMWARE_UPDATE, [](HTTPRequest * req, HTTPResponse * res) {
    if (!web_admin_authenticate(req)) {
      return web_authenticate_request(req,res);
    }
    HTTPBodyParser *parser;
    std::string contentType = req->getHeader("Content-Type");
    size_t semicolonPos = contentType.find(";");
    if (semicolonPos != std::string::npos) {
      contentType = contentType.substr(0, semicolonPos);
    }
    if (contentType == "multipart/form-data") {
      parser = new HTTPMultipartBodyParser(req);
    } else {
      return web_send_error_client(req,res,"Invalid Content-Type");
    }
    while (parser->nextField()) {
      std::string name = parser->getFieldName();
      if (name != "update") {
        log_w("Skipping unexpected field");
        continue;
      }
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
      size_t upload_len = 0;
      while (!parser->endOfField()) {
        byte buf[512];
        size_t curr_len = parser->read(buf, sizeof(buf));
        if (Update.write(buf, curr_len) != curr_len) {
          Update.printError(Serial);
        }
        upload_len += curr_len;
      }
      if (Update.end(true)) { //true to set the size to the current progress
        log_i("Update Success: %u\nRebooting...\n", upload_len);
      } else {
        Update.printError(Serial);
      }
      return web_reset(req,res,Update.hasError() ? "FAILED" : "COMPLETED");
    }
    web_send_error_client(req,res,"Invalid upload");
  });
}
#endif

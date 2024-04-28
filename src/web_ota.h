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

namespace WebOTA {

#ifndef UPDATE_SIZE_UNKNOWN
#define UPDATE_SIZE_UNKNOWN 0x00100000
#endif

WebGUI* _web_gui;
String _web_uri;

void setup(WebGUI* web_gui, const String& uri) {
  _web_gui = web_gui;
  _web_uri = uri;
  _web_gui->handle(HTTP_GET, _web_uri, [](HTTPRequest* req, HTTPResponse* res) {
    if (web_authenticateAdmin(req,res)) {
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
    _web_gui->sendPage(req,res,title,html);
  });
  _web_gui->handle(HTTP_POST, _web_uri, [](HTTPRequest* req, HTTPResponse* res) {
    if (web_authenticateAdmin(req,res)) {
      return;
    }
    HTTPBodyParser* parser;
    std::string contentType = req->getHeader("Content-Type");
    size_t semicolonPos = contentType.find(";");
    if (semicolonPos != std::string::npos) {
      contentType = contentType.substr(0, semicolonPos);
    }
    if (contentType == "multipart/form-data") {
      parser = new HTTPMultipartBodyParser(req);
    } else {
      return _web_gui->sendErrorClient(req,res,"Invalid Content-Type");
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
    _web_gui->sendErrorClient(req,res,"Invalid upload");
  });
};

const String& getUri() {
  return _web_uri;
};

}


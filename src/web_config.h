#pragma once

/**
 * Contiene variabili, tipi e funzioni per la configurazione del Web Server.
 * 
 * @see https://github.com/arduino-libraries/Arduino_JSON/blob/master/src/JSON.h
 */

#include "config.h"
#include "web_gui.h"
#include "web_reset.h"
#include <Arduino_JSON.h>

namespace WebConfig {

#define WEB_CONFIG_PATH_RESET   "/reset"
#define WEB_CONFIG_PATH_UPLOAD  "/upload"

WebGUI* _web_gui;
WebReset* _web_reset;
String _web_uri;
bool _config_publish;

void __web_config_handle_value_export(const char* key, const Config& config, JSONVar* json_export) {
  if (!preferences.isKey(key)) {
    return;
  }
  switch (config.type) {
    case UINT8:
      (*json_export)[key] = preferences.getUChar(key);
      break;
    case UINT16:
      (*json_export)[key] = preferences.getUShort(key);
      break;
    case UINT32:
      (*json_export)[key] = preferences.getULong(key);
      break;
    case UINT64:
      (*json_export)[key] = uint64_to_string(preferences.getULong64(key));
      break;
    case INT8:
      (*json_export)[key] = preferences.getChar(key);
      break;
    case INT16:
      (*json_export)[key] = preferences.getShort(key);
      break;
    case INT32:
      (*json_export)[key] = preferences.getLong(key);
      break;
    case INT64:
      (*json_export)[key] = int64_to_string(preferences.getLong64(key));
      break;
    case STRING:
      (*json_export)[key] = preferences.getString(key);
      break;
    case BOOL:
      (*json_export)[key] = preferences.getBool(key) ? true : false;
      break;
    case FLOAT:
      (*json_export)[key] = preferences.getFloat(key);
      break;
    case DOUBLE:
      (*json_export)[key] = preferences.getDouble(key);
      break;
    case STRUCT:
      // funziona solo perchè c'è una sola STRUCT (rules)
      static JSONVar v = JSON.parse(preferences.getString(key));
      (*json_export)[key] = v;
      break;
  }
}
  
void __web_config_handle_value_import(const char* key, const Config& config, JSONVar* json_import) {
  if (!(json_import->hasOwnProperty(key))) {
    return;
  }
  switch (config.type) {
    case UINT8:
      preferences.putUChar(key,(unsigned char) (*json_import)[key]);
      break;
    case UINT16:
      preferences.putUShort(key,(unsigned short) (*json_import)[key]);
      break;
    case UINT32:
      preferences.putULong(key,(unsigned long) (*json_import)[key]);
      break;
    case UINT64:
      preferences.putULong64(key,str_to_uint64((const char*) (*json_import)[key]));
      break;
    case INT8:
      preferences.putChar(key,(char) (*json_import)[key]);
      break;
    case INT16:
      preferences.putShort(key,(short) (*json_import)[key]);
      break;
    case INT32:
      preferences.putLong(key,(long) (*json_import)[key]);
      break;
    case INT64:
      preferences.putLong64(key,str_to_int64((const char*) (*json_import)[key]));
      break;
    case STRING:
      preferences.putString(key,(const char*) (*json_import)[key]);
      break;
    case BOOL:
      preferences.putBool(key,(bool) (*json_import)[key]);
      break;
    case FLOAT:
      preferences.putFloat(key,(double) (*json_import)[key]);
      break;
    case DOUBLE:
      preferences.putDouble(key,(double) (*json_import)[key]);
      break;
    case STRUCT:
      preferences.putString(key,JSON.stringify((*json_import)[key]));
      break;
  }
}

void __web_config_handle_change(HTTPRequest* req,HTTPResponse* res) {
  if (web_authenticateAdmin(req,res)) {
    return;
  }
  String param_name = "";
  String param_value = "";
  bool is_reset = false;
  bool is_download = false;
  HTTPBodyParser* parser = NULL;
  if (req->getHeader("Content-Type") == "application/x-www-form-urlencoded") {
    parser = new HTTPURLEncodedBodyParser(req);
  }
  if (parser != NULL) {
    while (parser->nextField()) {
      std::string name = parser->getFieldName();
      if (name == "name") {
        param_name = web_getParameter(parser);
      }
      if (name == "value") {
        param_value = web_getParameter(parser);
      }
      if (name == "reset") {
        is_reset = web_getParameter(parser).equals("true");
      }
      if (name == "download") {
        is_download = web_getParameter(parser).equals("true");
      }
    }
  } else {
    param_name = web_getParameter(req, "name");
    param_value = web_getParameter(req, "value");
    is_reset = web_getParameter(req, "reset").equals("true");
    is_download = web_getParameter(req, "download").equals("true");
  }
  String title = _web_gui->getTitle();
  const Config* config_selected = NULL;
  if (param_name != "") {
    for (int i = 0; i < len_array(config_defs); i++) {
      if (config_defs[i].type != DARRAY) {
        if (param_name == config_defs[i].key) {
          config_selected = &config_defs[i];
          break;
        }
      } else {
        for (int j = 1; j <= config_defs[i].count; j++) {
          for (int k = 0; k < config_defs[i].refs_len; k++) {
            String config_key = config_defs[i].key+String(j)+config_defs[i].refs[k].key;
            if (param_name == config_key) {
              config_selected = &config_defs[i].refs[k];
              goto label_web_handle_config_1;
            }
          }
        }
      }
    }
  }
  label_web_handle_config_1:
  String html = "<body style=\"text-align:center\"><h1>"+title+"</h1><h2>Configuration</h2>";
  if (config_selected == NULL) {
    html += "<table style=\"margin:auto\">";
    html += "<tr><th>key</th><th>type</th><th>value</th><th></th></tr>";
    JSONVar json_export;
    for (int i = 0; i < len_array(config_defs); i++) {
      if (config_defs[i].type != DARRAY) {
        if (is_download) {
          __web_config_handle_value_export(config_defs[i].key,config_defs[i],&json_export);
        } else {
          html += "<tr><td>"+String(config_defs[i].key)+"</td><td>"+String(ctype_str(config_defs[i].type))+"</td>";
          html += "<td><pre>"+html_encode(preferences_get(config_defs[i].key,config_defs[i].type,true))+"</pre></td>";
          html += "<td><button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='"+_web_uri+"?name="+String(config_defs[i].key)+"'\">Edit</button></td></tr>";
        }
      } else {
        for (int j = 1; j <= config_defs[i].count; j++) {
          for (int k = 0; k < config_defs[i].refs_len; k++) {
            String config_key_str = config_defs[i].key+String(j)+config_defs[i].refs[k].key;
            const char* config_key = config_key_str.c_str();
            if (is_download) {
              __web_config_handle_value_export(config_key,config_defs[i].refs[k],&json_export);
            } else {
              html += "<tr><td>"+config_key_str+"</td><td>"+String(ctype_str(config_defs[i].refs[k].type))+"</td>";
              html += "<td><pre>"+html_encode(preferences_get(config_key,config_defs[i].refs[k].type,true))+"</pre></td>";
              html += "<td><button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='"+_web_uri+"?name="+config_key_str+"'\">Edit</button></td></tr>";
            }
          }
        }
      }
    }
    if (is_download) {
      return web_sendResponse(req,res,200,"application/json",JSON.stringify(json_export),"config.json");
    }
    html += "</table>";
    html += "<form action=\""+_web_reset->getUri()+"\" method=\"POST\"><p>";
    html += "<button type=\"submit\" class=\"btn btn-primary\" onclick=\"return confirm('Are you sure?')\">Restart</button> ";
    html += "<button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='/'\">Cancel</button>";
    html += "</p></form>";
    html += "<form action=\""+_web_uri+WEB_CONFIG_PATH_RESET+"\" method=\"POST\"><p>";
    html += "<button type=\"button\" class=\"btn btn-success\" onclick=\"location='"+_web_uri+"?download=true'\">Download</button> ";
    html += "<button type=\"button\" class=\"btn btn-danger\" onclick=\"location='"+_web_uri+WEB_CONFIG_PATH_UPLOAD+"'\">Upload</button> ";
    html += "<button type=\"submit\" class=\"btn btn-danger\" onclick=\"return confirm('Are you sure?')\">Reset</button> ";
    html += "</p></form>";
    html += _web_gui->getFooter(true);
  } else if (!web_isRequestMethodPost(req)) {
    String config_value = preferences_get(param_name.c_str(),config_selected->type,true);
    html += "<p>"+param_name+"</p>";
    html += "<form action=\""+_web_uri+"\" method=\"POST\"><p>";
    html += "<input type=\"hidden\" name=\"name\" value=\""+html_encode(param_name)+"\" />";
    html += html_input(config_selected->type,"value",config_value);
    html += "</p>";
    if (config_selected->desc != NULL) {
      html += "<pre>" + String(config_selected->desc) + "</pre>";
    }
    html += "<p>";
    html += "<input type=\"hidden\" name=\"reset\" value=\"false\" />";
    html += "<button type=\"submit\" class=\"btn btn-primary\">Save</button> ";
    html += "<button type=\"submit\" class=\"btn btn-danger\" onclick=\"this.form.reset.value = 'true'\">Reset</button> ";
    html += "<button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='"+_web_uri+"'\">Cancel</button>";
    html += "</p></form>";
  } else {
    String publish_key = "";
    if (_config_publish) {
      publish_key = "config."+param_name;
    }
    if (is_reset) {
      preferences_remove(param_name.c_str(),config_selected->type,publish_key);
    } else {
      preferences_put(param_name.c_str(),config_selected->type,param_value,publish_key);
    }
    web_sendRedirect(req,res,_web_uri);
    return;
  }
  html += "</body>";
  _web_gui->sendPage(req,res,title,html);
}

void __web_config_handle_upload(HTTPRequest* req,HTTPResponse* res) {
  if (web_authenticateAdmin(req,res)) {
    return;
  }
  if (!web_isRequestMethodPost(req)) {
    String title = _web_gui->getTitle();
    String html = "<body style=\"text-align:center\"><h1>"+title+"</h1><h2>Upload Configurations</h2>";
    html += "<form action=\""+_web_uri+WEB_CONFIG_PATH_UPLOAD+"\" method=\"POST\" enctype=\"multipart/form-data\"><p>";
    html += "<input type=\"file\" name=\"upload\" accept=\".json,application/json\" />";
    html += "</p><p>";
    html += "<button type=\"submit\" class=\"btn btn-primary\">Upload</button> ";
    html += "<button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='"+_web_uri+"'\">Cancel</button>";
    html += "</p></form>";
    html += "</body>";
    return _web_gui->sendPage(req,res,title,html);
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
    if (name != "upload") {
      log_w("Skipping unexpected field");
      continue;
    }
    std::string filename = parser->getFieldFilename();
    log_i("Upload: %s", filename.c_str());
    uint8_t upload_buf[CONF_WEB_UPLOAD_LIMIT];
    size_t upload_len = 0;
    while (!parser->endOfField()) {
      byte buf[512];
      size_t curr_len = parser->read(buf, sizeof(buf));
      if (upload_len + curr_len + 1 < CONF_WEB_UPLOAD_LIMIT) {
        memcpy(&upload_buf[upload_len],buf,curr_len * sizeof(uint8_t));
        upload_len += curr_len;
      }
    }
    log_i("Upload Success: %u", upload_len);
    upload_buf[upload_len] = 0;
    JSONVar json_import = JSON.parse(String((char*)upload_buf));
    for (int i = 0; i < len_array(config_defs); i++) {
      if (config_defs[i].type != DARRAY) {
        __web_config_handle_value_import(config_defs[i].key,config_defs[i],&json_import);
      } else {
        for (int j = 1; j <= config_defs[i].count; j++) {
          for (int k = 0; k < config_defs[i].refs_len; k++) {
            String config_key = config_defs[i].key+String(j)+config_defs[i].refs[k].key;
            __web_config_handle_value_import(config_key.c_str(),config_defs[i].refs[k],&json_import);
          }
        }
      }
    }
    return;
  }
  _web_gui->sendErrorClient(req,res,"Invalid upload");
}

void setup(WebGUI* web_gui, WebReset* web_reset, const String& uri, bool config_publish = false) {
  _web_gui = web_gui;
  _web_reset = web_reset;
  _web_uri = uri;
  _config_publish = config_publish;
  _web_gui->handle(HTTP_ANY, _web_uri, __web_config_handle_change);
  _web_gui->handle(HTTP_ANY, _web_uri + WEB_CONFIG_PATH_UPLOAD, __web_config_handle_upload);
  _web_gui->handle(HTTP_ANY, _web_uri + WEB_CONFIG_PATH_RESET, [](HTTPRequest* req, HTTPResponse* res) {
    if (web_authenticateAdmin(req,res)) {
      return;
    }
    if (web_isRequestMethodPost(req)) {
      log_i("preferences clear");
      preferences.clear();
    }
    web_sendRedirect(req,res,_web_uri);
  });
}

}


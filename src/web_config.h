#ifndef INCLUDE_WEB_CONFIG_H
#define INCLUDE_WEB_CONFIG_H

/**
 * Contiene variabili, tipi e funzioni per la configurazione del Web Server.
 * 
 * @see https://github.com/arduino-libraries/Arduino_JSON/blob/master/src/JSON.h
 */

#include "config.h"
#include "web.h"
#include "web_admin.h"
#include "web_reset.h"
#include "web_templates.h"
#include <Arduino_JSON.h>

bool web_config_publish;

void web_config_handle_value_export(const char * key, Config config, JSONVar * json_export) {
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
      static JSONVar v = JSON.parse(preferences.getString(key).c_str());
      (*json_export)[key] = v;
      break;
  }
}
  
void web_config_handle_value_import(const char * key, Config config, JSONVar * json_import) {
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

void web_config_handle_change(HTTPRequest * req, HTTPResponse * res) {
  if (!web_admin_authenticate(req)) {
    return web_authenticate_request(req,res);
  }
  String param_name = "";
  String param_value = "";
  bool is_reset = false;
  bool is_download = false;
  HTTPBodyParser *parser = NULL;
  if (req->getHeader("Content-Type") == "application/x-www-form-urlencoded") {
    parser = new HTTPURLEncodedBodyParser(req);
  }
  if (parser != NULL) {
    while (parser->nextField()) {
      std::string name = parser->getFieldName();
      if (name == "name") {
        param_name = web_parameter(parser);
      }
      if (name == "value") {
        param_value = web_parameter(parser);
      }
      if (name == "reset") {
        is_reset = web_parameter(parser).equals("true");
      }
      if (name == "download") {
        is_download = web_parameter(parser).equals("true");
      }
    }
  } else {
    param_name = web_parameter(req, "name");
    param_value = web_parameter(req, "value");
    is_reset = web_parameter(req, "reset").equals("true");
    is_download = web_parameter(req, "download").equals("true");
  }
  String title = web_html_title();
  const Config * config_selected = NULL;
  if (param_name != "") {
    for (int i = 0; i < len(config_defs); i++) {
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
    for (int i = 0; i < len(config_defs); i++) {
      if (config_defs[i].type != DARRAY) {
        if (is_download) {
          web_config_handle_value_export(config_defs[i].key,config_defs[i],&json_export);
        } else {
          html += "<tr><td>"+String(config_defs[i].key)+"</td><td>"+String(ctype_str(config_defs[i].type))+"</td>";
          html += "<td><pre>"+html_encode(preferences_get(config_defs[i].key,config_defs[i].type,true))+"</pre></td>";
          html += "<td><button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='"+String(CONF_WEB_URI_CONFIG)+"?name="+String(config_defs[i].key)+"'\">Edit</button></td></tr>";
        }
      } else {
        for (int j = 1; j <= config_defs[i].count; j++) {
          for (int k = 0; k < config_defs[i].refs_len; k++) {
            String config_key_str = config_defs[i].key+String(j)+config_defs[i].refs[k].key;
            const char * config_key = config_key_str.c_str();
            if (is_download) {
              web_config_handle_value_export(config_key,config_defs[i].refs[k],&json_export);
            } else {
              html += "<tr><td>"+config_key_str+"</td><td>"+String(ctype_str(config_defs[i].refs[k].type))+"</td>";
              html += "<td><pre>"+html_encode(preferences_get(config_key,config_defs[i].refs[k].type,true))+"</pre></td>";
              html += "<td><button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='"+String(CONF_WEB_URI_CONFIG)+"?name="+config_key_str+"'\">Edit</button></td></tr>";
            }
          }
        }
      }
    }
    if (is_download) {
      return web_download_text(req,res,"application/json","config.json",JSON.stringify(json_export));
    }
    html += "</table>";
    html += "<form action=\""+String(CONF_WEB_URI_RESET)+"\" method=\"POST\"><p>";
    html += "<button type=\"submit\" class=\"btn btn-primary\" onclick=\"return confirm('Are you sure?')\">Restart</button> ";
    html += "<button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='/'\">Cancel</button>";
    html += "</p></form>";
    html += "<form action=\""+String(CONF_WEB_URI_CONFIG_RESET)+"\" method=\"POST\"><p>";
    html += "<button type=\"button\" class=\"btn btn-success\" onclick=\"location='"+String(CONF_WEB_URI_CONFIG)+"?download=true'\">Download</button> ";
    html += "<button type=\"button\" class=\"btn btn-danger\" onclick=\"location='"+String(CONF_WEB_URI_CONFIG_UPLOAD)+"'\">Upload</button> ";
    html += "<button type=\"submit\" class=\"btn btn-danger\" onclick=\"return confirm('Are you sure?')\">Reset</button> ";
    html += "</p></form><hr/>";
    html += web_html_footer(true);
  } else if (!web_request_post(req)) {
    String config_value = preferences_get(param_name.c_str(),config_selected->type,true);
    html += "<p>"+param_name+"</p>";
    html += "<form action=\""+String(CONF_WEB_URI_CONFIG)+"\" method=\"POST\"><p>";
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
    html += "<button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='"+String(CONF_WEB_URI_CONFIG)+"'\">Cancel</button>";
    html += "</p></form>";
  } else {
    String publish_key = "";
    if (web_config_publish) {
      publish_key = "config."+param_name;
    }
    if (is_reset) {
      preferences_remove(param_name.c_str(),config_selected->type,publish_key);
    } else {
      preferences_put(param_name.c_str(),config_selected->type,param_value,publish_key);
    }
    return web_send_redirect(req,res,CONF_WEB_URI_CONFIG);
  }
  html += "</body>";
  web_send_page(req,res,title,html);
}

void web_config_handle_upload(HTTPRequest * req, HTTPResponse * res) {
  if (!web_admin_authenticate(req)) {
    return web_authenticate_request(req,res);
  }
  if (!web_request_post(req)) {
    String title = web_html_title();
    String html = "<body style=\"text-align:center\"><h1>"+title+"</h1><h2>Upload Configurations</h2>";
    html += "<form action=\""+String(CONF_WEB_URI_CONFIG_UPLOAD)+"\" method=\"POST\" enctype=\"multipart/form-data\"><p>";
    html += "<input type=\"file\" name=\"upload\" accept=\".json,application/json\" />";
    html += "</p><p>";
    html += "<button type=\"submit\" class=\"btn btn-primary\">Upload</button> ";
    html += "<button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='"+String(CONF_WEB_URI_CONFIG)+"'\">Cancel</button>";
    html += "</p></form>";
    html += "</body>";
    return web_send_page(req,res,title,html);
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
    JSONVar json_import = JSON.parse(String((char *)upload_buf));
    for (int i = 0; i < len(config_defs); i++) {
      if (config_defs[i].type != DARRAY) {
        web_config_handle_value_import(config_defs[i].key,config_defs[i],&json_import);
      } else {
        for (int j = 1; j <= config_defs[i].count; j++) {
          for (int k = 0; k < config_defs[i].refs_len; k++) {
            String config_key = config_defs[i].key+String(j)+config_defs[i].refs[k].key;
            web_config_handle_value_import(config_key.c_str(),config_defs[i].refs[k],&json_import);
          }
        }
      }
    }
    return;
  }
  web_send_error_client(req,res,"Invalid upload");
}

void web_config_setup(bool config_publish) {
  web_config_publish = config_publish;
  web_reset_setup();
  web_server_register(HTTP_ANY, CONF_WEB_URI_CONFIG, &web_config_handle_change);
  web_server_register(HTTP_ANY, CONF_WEB_URI_CONFIG_UPLOAD, &web_config_handle_upload);
  web_server_register(HTTP_ANY, CONF_WEB_URI_CONFIG_RESET, [](HTTPRequest * req, HTTPResponse * res) {
    if (!web_admin_authenticate(req)) {
      return web_authenticate_request(req,res);
    }
    if (web_request_post(req)) {
      log_i("preferences clear");
      preferences.clear();
    }
    web_send_redirect(req,res,CONF_WEB_URI_CONFIG);
  });
}

#endif

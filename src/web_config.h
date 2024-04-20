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

void web_config_handle_change() {
  if (!web_admin_authenticate()) {
    return Web.authenticateRequest();
  }
  String param_name = Web.getParameter("name");
  String param_value = Web.getParameter("value");
  bool is_reset = Web.getParameter("reset").equals("true");
  bool is_download = Web.getParameter("download").equals("true");
  String title = web_html_title();
  const Config * config_selected = NULL;
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
      return Web.sendFile("application/json","config.json",JSON.stringify(json_export));
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
  } else if (!Web.isRequestMethodPost()) {
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
    return Web.sendRedirect(CONF_WEB_URI_CONFIG);
  }
  html += "</body>";
  web_send_page(title,html);
}

int config_upload_len;
uint8_t config_upload_buf[CONF_WEB_UPLOAD_LIMIT];

void web_handle_config_upload() {
  if (!web_admin_authenticate()) {
    return Web.authenticateRequest();
  }
  if (!Web.isRequestMethodPost()) {
    String title = web_html_title();
    String html = "<body style=\"text-align:center\"><h1>"+title+"</h1><h2>Upload Configurations</h2>";
    html += "<form action=\""+String(CONF_WEB_URI_CONFIG_UPLOAD)+"\" method=\"POST\" enctype=\"multipart/form-data\"><p>";
    html += "<input type=\"file\" name=\"upload\" accept=\".json,application/json\" />";
    html += "</p><p>";
    html += "<button type=\"submit\" class=\"btn btn-primary\">Upload</button> ";
    html += "<button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='"+String(CONF_WEB_URI_CONFIG)+"'\">Cancel</button>";
    html += "</p></form>";
    html += "</body>";
    return web_send_page(title,html);
  }
  HTTPUpload& upload = Web.getUpload();
  if (upload.status == UPLOAD_FILE_START) {
    log_i("Upload: %s", upload.filename.c_str());
    config_upload_len = 0;
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (config_upload_len + upload.currentSize + 1 < CONF_WEB_UPLOAD_LIMIT) {
      memcpy(&config_upload_buf[config_upload_len],upload.buf,upload.currentSize * sizeof(uint8_t));
      config_upload_len += upload.currentSize;
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    log_i("Upload Success: %u", upload.totalSize);
    config_upload_buf[config_upload_len] = 0;
    JSONVar json_import = JSON.parse(String((char *)config_upload_buf));
    config_upload_len = 0;
    for (int i = 0; i < len_array(config_defs); i++) {
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
  }
}

void web_config_setup(bool config_publish) {
  web_config_publish = config_publish;
  web_reset_setup();
  Web.handle(HTTP_ANY, CONF_WEB_URI_CONFIG, web_config_handle_change);
  Web.handle(HTTP_GET, CONF_WEB_URI_CONFIG_UPLOAD, web_handle_config_upload);
  Web.handleUpload(HTTP_POST, CONF_WEB_URI_CONFIG_UPLOAD, []() {
    Web.sendRedirect(CONF_WEB_URI_CONFIG);
  }, web_handle_config_upload);
  Web.handle(HTTP_ANY, CONF_WEB_URI_CONFIG_RESET, []() {
    if (!web_admin_authenticate()) {
      return Web.authenticateRequest();
    }
    if (Web.isRequestMethodPost()) {
      log_i("preferences clear");
      preferences.clear();
    }
    Web.sendRedirect(CONF_WEB_URI_CONFIG);
  });
}

#endif

#ifndef INCLUDE_WEB_CONFIG_H
#define INCLUDE_WEB_CONFIG_H

/**
 * Contiene variabili, tipi e funzioni per la configurazione del Web Server.
 * 
 * @see https://github.com/arduino-libraries/Arduino_JSON/blob/master/src/JSON.h
 */

#include "config.h"
#include "web_gui.h"
#include "web_reset.h"
#include <Arduino_JSON.h>

#define WEB_CONFIG_PATH_RESET   "/reset"
#define WEB_CONFIG_PATH_UPLOAD  "/upload"

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

class WebConfig : public WebAdminComponent {
private:
  WebGUI* _web_gui;
  WebReset* _web_reset;
  String _web_uri;
  bool _config_publish;
  void _handle_change();
  int _upload_len;
  uint8_t _upload_buf[CONF_WEB_UPLOAD_LIMIT];
  void _handle_upload();
public:
  WebConfig(WebGUI* web_gui, WebReset* web_reset, const String& uri, bool (*web_admin_authenticate)() = nullptr, bool config_publish = false);
};

void WebConfig::_handle_change() {
  if (authenticateAdmin()) {
    return;
  }
  String param_name = _web_gui->getParameter("name");
  String param_value = _web_gui->getParameter("value");
  bool is_reset = _web_gui->getParameter("reset").equals("true");
  bool is_download = _web_gui->getParameter("download").equals("true");
  String title = _web_gui->getTitle();
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
          html += "<td><button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='"+_web_uri+"?name="+String(config_defs[i].key)+"'\">Edit</button></td></tr>";
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
              html += "<td><button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='"+_web_uri+"?name="+config_key_str+"'\">Edit</button></td></tr>";
            }
          }
        }
      }
    }
    if (is_download) {
      return _web_gui->sendResponse(200,"application/json",JSON.stringify(json_export),"config.json");
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
  } else if (!_web_gui->isRequestMethodPost()) {
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
    return _web_gui->sendRedirect(_web_uri);
  }
  html += "</body>";
  _web_gui->sendPage(title,html);
}

void WebConfig::_handle_upload() {
  if (authenticateAdmin()) {
    return;
  }
  if (!_web_gui->isRequestMethodPost()) {
    String title = _web_gui->getTitle();
    String html = "<body style=\"text-align:center\"><h1>"+title+"</h1><h2>Upload Configurations</h2>";
    html += "<form action=\""+_web_uri+WEB_CONFIG_PATH_UPLOAD+"\" method=\"POST\" enctype=\"multipart/form-data\"><p>";
    html += "<input type=\"file\" name=\"upload\" accept=\".json,application/json\" />";
    html += "</p><p>";
    html += "<button type=\"submit\" class=\"btn btn-primary\">Upload</button> ";
    html += "<button type=\"button\" class=\"btn btn-secondary\" onclick=\"location='"+_web_uri+"'\">Cancel</button>";
    html += "</p></form>";
    html += "</body>";
    return _web_gui->sendPage(title,html);
  }
  HTTPUpload& upload = _web_gui->getUpload();
  if (upload.status == UPLOAD_FILE_START) {
    log_i("Upload: %s", upload.filename.c_str());
    _upload_len = 0;
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (_upload_len + upload.currentSize + 1 < CONF_WEB_UPLOAD_LIMIT) {
      memcpy(&_upload_buf[_upload_len],upload.buf,upload.currentSize * sizeof(uint8_t));
      _upload_len += upload.currentSize;
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    log_i("Upload Success: %u", upload.totalSize);
    _upload_buf[_upload_len] = 0;
    JSONVar json_import = JSON.parse(String((char *)_upload_buf));
    _upload_len = 0;
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

WebConfig::WebConfig(WebGUI* web_gui, WebReset* web_reset, const String& uri, bool (*web_admin_authenticate)(), bool config_publish) : WebAdminComponent(web_gui,web_admin_authenticate) {
  _web_gui = web_gui;
  _web_reset = web_reset;
  _web_uri = uri;
  _config_publish = config_publish;
  _web_gui->handle(HTTP_ANY, _web_uri, [this]() {
    _handle_change();
  });
  _web_gui->handle(HTTP_GET, _web_uri + WEB_CONFIG_PATH_UPLOAD, [this]() {
    _handle_upload();
  });
  _web_gui->handleUpload(HTTP_POST, _web_uri + WEB_CONFIG_PATH_UPLOAD, [this]() {
    _web_gui->sendRedirect(_web_uri);
  }, [this]() {
    _handle_upload();
  });
  _web_gui->handle(HTTP_ANY, _web_uri + WEB_CONFIG_PATH_RESET, [this]() {
    if (authenticateAdmin()) {
      return;
    }
    if (_web_gui->isRequestMethodPost()) {
      log_i("preferences clear");
      preferences.clear();
    }
    _web_gui->sendRedirect(_web_uri);
  });
}

#endif

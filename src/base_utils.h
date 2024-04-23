#ifndef INCLUDE_BASE_UTILS_H
#define INCLUDE_BASE_UTILS_H

/**
 * Contiene tipi e funzioni di uso generico.
 */

#include "base_platform.h"
#include <Arduino.h>
#include <Arduino_JSON.h>
#include <base64.h>
#ifdef PLATFORM_ESP32
extern "C" {
#include "crypto/base64.h"
}
#endif

const PROGMEM char * EMPTY = "";

typedef enum {
  UINT8  = 16,
  UINT16 = 17,
  UINT32 = 18,
  UINT64 = 19,
  INT8   = 20,
  INT16  = 21,
  INT32  = 22,
  INT64  = 23,
  STRING = 24,
  BOOL   = 25,
  FLOAT  = 26,
  DOUBLE = 27,
  DARRAY = 28,
  STRUCT = 29
} ctype_t;

const char * ctype_str(ctype_t type) {
  switch (type) {
    case UINT8:  return "uint8";
    case UINT16: return "uint16";
    case UINT32: return "uint32";
    case UINT64: return "uint64";
    case INT8:   return "int8";
    case INT16:  return "int16";
    case INT32:  return "int32";
    case INT64:  return "int64";
    case STRING: return "string";
    case BOOL:   return "bool";
    case FLOAT:  return "float";
    case DOUBLE: return "double";
    case DARRAY: return "darray";
    case STRUCT: return "struct";
    default: return NULL;
  }
}

String uint64_to_string(uint64_t num) {
  if (num == 0) {
    return "0";
  }
  String str = EMPTY;
  while (num > 0) {
    str = String((uint8_t)(num%10)) + str;
    num = num / 10;
  }
  return str;
}

String int64_to_string(int64_t num) {
  if (num == 0) {
    return "0";
  }
  bool negative = num < 0;
  if (negative) {
    num = -num;
  }
  String str = EMPTY;
  while (num > 0) {
    str = String((uint8_t)(num%10)) + str;
    num = num / 10;
  }
  if (negative) {
    str = "-" + str;
  }
  return str;
}

uint8_t str_to_uint8(const char * str) {
  uint8_t number;
  sscanf(str, "%hhu", &number);
  return number;
}

uint16_t str_to_uint16(const char * str) {
  uint16_t number;
  sscanf(str, "%" SCNu16, &number);
  return number;
}

uint32_t str_to_uint32(const char * str) {
  uint32_t number;
  sscanf(str, "%" SCNu32, &number);
  return number;
}

uint64_t str_to_uint64(const char * str) {
  uint64_t number;
  sscanf(str, "%" SCNu64, &number);
  return number;
}

int8_t str_to_int8(const char * str) {
  int8_t number;
  sscanf(str, "%hhd", &number);
  return number;
}

int16_t str_to_int16(const char * str) {
  int16_t number;
  sscanf(str, "%" SCNi16, &number);
  return number;
}

int32_t str_to_int32(const char * str) {
  int32_t number;
  sscanf(str, "%" SCNi32, &number);
  return number;
}

int64_t str_to_int64(const char * str) {
  int64_t number;
  sscanf(str, "%" SCNi64, &number);
  return number;
}

bool str_to_bool(const char * str) {
  bool number = false;
  if (String(str) == "true") {
    number = true;
  }
  return number;
}

float str_to_float(const char * str) {
  float number;
  sscanf(str, "%f", &number);
  return number;
}

double str_to_double(const char * str) {
  double number;
  sscanf(str, "%lf", &number);
  return number;
}

uint8_t str_to_uint8(String str) {
  return str_to_uint8(str.c_str());
}

uint16_t str_to_uint16(String str) {
  return str_to_uint16(str.c_str());
}

uint32_t str_to_uint32(String str) {
  return str_to_uint32(str.c_str());
}

uint64_t str_to_uint64(String str) {
  return str_to_uint64(str.c_str());
}

int8_t str_to_int8(String str) {
  return str_to_int8(str.c_str());
}

int16_t str_to_int16(String str) {
  return str_to_int16(str.c_str());
}

int32_t str_to_int32(String str) {
  return str_to_int32(str.c_str());
}

int64_t str_to_int64(String str) {
  return str_to_int64(str.c_str());
}

bool str_to_bool(String str) {
  return str_to_bool(str.c_str());
}

float str_to_float(String str) {
  return str_to_float(str.c_str());
}

double str_to_double(String str) {
  return str_to_double(str.c_str());
}

bool at_interval(uint32_t interval, uint32_t from = 0) {
  return interval > 0 && millis() - from > interval;
}

void items_publish(JSONVar message);

void item_publish(const char * name, String value) {
  JSONVar message;
  message[name] = value;
  items_publish(message);
}

void item_publish(const char * name, int32_t value) {
  JSONVar message;
  message[name] = value;
  items_publish(message);
}

void item_publish(const char * name, uint32_t value) {
  JSONVar message;
  message[name] = value;
  items_publish(message);
}

void item_publish(const char * name, bool value) {
  JSONVar message;
  message[name] = value;
  items_publish(message);
}

void item_publish(const char * name, double value) {
  JSONVar message;
  message[name] = value;
  items_publish(message);
}

String html_encode(String str) {
  String buffer;
  buffer.reserve(str.length());
  for (size_t pos = 0; pos != str.length(); ++pos) {
    switch(str[pos]) {
      case '&':  buffer += "&amp;";         break;
      case '\"': buffer += "&quot;";        break;
      case '\'': buffer += "&apos;";        break;
      case '<':  buffer += "&lt;";          break;
      case '>':  buffer += "&gt;";          break;
      default:   buffer += str.charAt(pos); break;
    }
  }
  return buffer;
}

String html_input(ctype_t type, String name = "value", String value = EMPTY, String attrs = EMPTY) {
  if (type == BOOL) {
    String input_options = EMPTY;
    if (value == "true") {
      input_options = "selected=\"selected\"";
    }
    String html = "<select name=\""+name+"\" required=\"required\" "+attrs+">";
    html += "<option value=\"false\">false</option>";
    html += "<option value=\"true\" "+input_options+">true</option>";
    html += "</select>";
    return html;
  } else if (type == STRUCT) {
    String html = "<textarea name=\""+name+"\" cols=\"32\" rows=\"8\" "+attrs+" >";
    html += html_encode(value);
    html += "</textarea>";
    return html;
  } else {
    String input_type = "number";
    String input_options = "required=\"required\"";
    if (type == UINT8) {
      input_options += " min=\"0\" max=\"255\"";
    } else if (type == UINT16) {
      input_options += " min=\"0\" max=\"65535\"";
    } else if (type == UINT32) {
      input_options += " min=\"0\" max=\"4294967295\"";
    } else if (type == UINT64) {
      input_options += " min=\"0\"";
    } else if (type == INT8) {
      input_options += " min=\"-128\" max=\"127\"";
    } else if (type == INT16) {
      input_options += " min=\"-32768\" max=\"32767\"";
    } else if (type == INT32) {
      input_options += " min=\"-2147483648\" max=\"2147483647\"";
    } else if (type == INT64) {
    } else if (type == FLOAT || type == DOUBLE) {
      input_options += " step=\"any\"";
    } else {
      input_type = "text";
      input_options = EMPTY;
    }
    return "<input type=\""+input_type+"\" name=\""+name+"\" value=\""+html_encode(value)+"\" "+input_options+" "+attrs+"/>";
  }
}

String base64_encode_str(String input) {
  return base64::encode(input);
}

#ifdef PLATFORM_ESP32
String base64_decode_str(String input) {
  const char *base64String = input.c_str();
  size_t length;
  unsigned char *array = base64_decode(reinterpret_cast<const unsigned char *>(base64String), strlen(base64String), &length);
  String result = "";
  for (size_t i = 0; i < length; i++) {
    result += (char)array[i];
  }
  return result;
  return "";
}
#else
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64_index(char c) {
  if (c >= 'A' && c <= 'Z')
    return c - 'A';
  if (c >= 'a' && c <= 'z')
    return c - 'a' + 26;
  if (c >= '0' && c <= '9')
    return c - '0' + 52;
  if (c == '+')
    return 62;
  if (c == '/')
    return 63;
  return -1;
}

String base64_decode_str(String encodedString) {
  String decodedString = "";
  size_t len = encodedString.length();

  for (size_t i = 0; i < len; i += 4) {
    unsigned char a = base64_index(encodedString[i]);
    unsigned char b = base64_index(encodedString[i + 1]);
    unsigned char c = base64_index(encodedString[i + 2]);
    unsigned char d = base64_index(encodedString[i + 3]);

    decodedString += (a << 2) | (b >> 4);
    if (encodedString[i + 2] != '=')
      decodedString += ((b & 15) << 4) | (c >> 2);
    if (encodedString[i + 3] != '=')
      decodedString += ((c & 3) << 6) | d;
  }

  return decodedString;
}
#endif

#endif

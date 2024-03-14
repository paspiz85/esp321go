#ifndef INCLUDE_RULES_H
#define INCLUDE_RULES_H

#include <Arduino.h>
#include <Arduino_JSON.h>

// https://github.com/arduino-libraries/Arduino_JSON/blob/master/src/JSON.h

typedef enum {
  TRIGGER_PIN = 1,
  TRIGGER_OUTPUT = 2
} trigger_source_t;

typedef struct rule {
  uint8_t num;
  JSONVar config;
  JSONVar context;
} Rule;

typedef struct trigger_spec {
  trigger_source_t source_type;
  uint8_t source_num;
  input_type_t input_type;
  bool change;
} TriggerSpec;

typedef struct trigger {
  Rule* rule;
  TriggerSpec spec; std::function<void(Rule*,TriggerSpec*,int,double,input_type_t,bool)> handler;
} Trigger;

uint8_t rules_size = 0;
Rule rules[CONF_RULES_TRIGGERS_SIZE];

uint8_t triggers_size = 0;
Trigger triggers[CONF_RULES_TRIGGERS_SIZE];
bool pin_triggers[CONF_SCHEMA_PIN_COUNT][CONF_RULES_TRIGGERS_SIZE];
bool output_triggers[CONF_SCHEMA_OUTPUT_COUNT][CONF_RULES_TRIGGERS_SIZE];

void on_pin_read(uint8_t pin,int value,input_type_t itype,bool change) {
  for (uint8_t j = 0; j != triggers_size; j++) {
    if (pin_triggers[pin][j]) {
      triggers[j].handler(triggers[j].rule,&(triggers[j].spec),value,value,itype,change);
    }
  }
}

void on_pin_read(uint8_t pin,double value,input_type_t itype,bool change) {
  for (uint8_t j = 0; j != triggers_size; j++) {
    if (pin_triggers[pin][j]) {
      triggers[j].handler(triggers[j].rule,&(triggers[j].spec),value,value,itype,change);
    }
  }
}

void on_output_write(uint8_t num,bool reset) {
  for (uint8_t j = 0; j != triggers_size; j++) {
    if (output_triggers[num-1][j]) {
      triggers[j].handler(triggers[j].rule,&(triggers[j].spec),0,0,NO_INPUT,false);
    }
  }
}

bool rules_condition_eval(Rule* rule,JSONVar* condition,TriggerSpec* spec,int value,double fvalue) {
  if (JSON.typeof((*condition)["type"]) != "string") {
    log_d("rule %d condition dont have type",rule->num);
    return false;
  }
  String type = (*condition)["type"];
  String str = "";
  if (type == "match_str") {
    if (JSON.typeof((*condition)["input"]) == "number") {
      int input_num = (*condition)["input"];
      Input input = inputs[input_num-1];
      str = input_get(&input,true);
    } else if (JSON.typeof((*condition)["output"]) == "number") {
      int output_num = (*condition)["output"];
      Output output = outputs[output_num-1];
      str = output_get(&output);
    } else {
      str = String(value);
    }
    if (JSON.typeof((*condition)["value"]) == "string") {
      String match = (*condition)["value"];
      return str == match;
    } else if (JSON.typeof((*condition)["value_input"]) == "number") {
      int input_num = (*condition)["value_input"];
      Input input = inputs[input_num-1];
      String match = input_get(&input,true);
      return str == match;
    } else if (JSON.typeof((*condition)["value_output"]) == "number") {
      int output_num = (*condition)["value_output"];
      Output output = outputs[output_num-1];
      String match = output_get(&output);
      return str == match;
    }
    return false;
  }
  if (type == "match") {
    if (JSON.typeof((*condition)["input"]) == "number") {
      int input_num = (*condition)["input"];
      Input input = inputs[input_num-1];
      str = input_get(&input,true);
      value = str.toInt();
    } else if (JSON.typeof((*condition)["output"]) == "number") {
      int output_num = (*condition)["output"];
      Output output = outputs[output_num-1];
      str = output_get(&output);
      value = str.toInt();
    }
    if (JSON.typeof((*condition)["value"]) == "number") {
      int match = (*condition)["value"];
      return value == match;
    } else if (JSON.typeof((*condition)["value_input"]) == "number") {
      int input_num = (*condition)["value_input"];
      Input input = inputs[input_num-1];
      str = input_get(&input,true);
      int match = str.toInt();
      return value == match;
    } else if (JSON.typeof((*condition)["value_output"]) == "number") {
      int output_num = (*condition)["value_output"];
      Output output = outputs[output_num-1];
      str = output_get(&output);
      int match = str.toInt();
      return value == match;
    }
    return false;
  }
  if (type == "match_double") {
    if (JSON.typeof((*condition)["input"]) == "number") {
      int input_num = (*condition)["input"];
      Input input = inputs[input_num-1];
      str = input_get(&input,true);
      fvalue = str.toDouble();
    } else if (JSON.typeof((*condition)["output"]) == "number") {
      int output_num = (*condition)["output"];
      Output output = outputs[output_num-1];
      if (output.type == DOUBLE_VAR) {
        fvalue = preferences.getDouble(output.key.c_str());
      } if (output.type == FLOAT_VAR) {
        fvalue = (double) preferences.getFloat(output.key.c_str());
      } else {
        str = output_get(&output);
        fvalue = str.toDouble();
      }
    }
    if (JSON.typeof((*condition)["value"]) == "number") {
      double match = (*condition)["value"];
      return fvalue == match;
    } else if (JSON.typeof((*condition)["value_input"]) == "number") {
      int input_num = (*condition)["value_input"];
      Input input = inputs[input_num-1];
      str = input_get(&input,true);
      double match = str.toDouble();
      return fvalue == match;
    } else if (JSON.typeof((*condition)["value_output"]) == "number") {
      int output_num = (*condition)["value_output"];
      Output output = outputs[output_num-1];
      double match = 0;
      if (output.type == DOUBLE_VAR) {
        match = preferences.getDouble(output.key.c_str());
      } else if (output.type == FLOAT_VAR) {
        match = preferences.getFloat(output.key.c_str());
      } else {
        str = output_get(&output);
        match = str.toDouble();
      }
      return fvalue == match;
    }
    return false;
  }
  if (type == "threshold") {
    if (JSON.typeof((*condition)["input"]) == "number") {
      int input_num = (*condition)["input"];
      Input input = inputs[input_num-1];
      str = input_get(&input,true);
      value = str.toInt();
    } else if (JSON.typeof((*condition)["output"]) == "number") {
      int output_num = (*condition)["output"];
      Output output = outputs[output_num-1];
      str = output_get(&output);
      value = str.toInt();
    }
    if (JSON.typeof((*condition)["value"]) == "number") {
      int match = (*condition)["value"];
      return value > match;
    } else if (JSON.typeof((*condition)["value_input"]) == "number") {
      int input_num = (*condition)["value_input"];
      Input input = inputs[input_num-1];
      str = input_get(&input,true);
      int match = str.toInt();
      return value > match;
    } else if (JSON.typeof((*condition)["value_output"]) == "number") {
      int output_num = (*condition)["value_output"];
      Output output = outputs[output_num-1];
      str = output_get(&output);
      int match = str.toInt();
      return value > match;
    }
    return false;
  }
  if (type == "threshold_double") {
    if (JSON.typeof((*condition)["input"]) == "number") {
      int input_num = (*condition)["input"];
      Input input = inputs[input_num-1];
      str = input_get(&input,true);
      fvalue = str.toDouble();
    } else if (JSON.typeof((*condition)["output"]) == "number") {
      int output_num = (*condition)["output"];
      Output output = outputs[output_num-1];
      if (output.type == DOUBLE_VAR) {
        fvalue = preferences.getDouble(output.key.c_str());
      } if (output.type == FLOAT_VAR) {
        fvalue = (double) preferences.getFloat(output.key.c_str());
      } else {
        str = output_get(&output);
        fvalue = str.toDouble();
      }
    }
    if (JSON.typeof((*condition)["value"]) == "number") {
      double match = (*condition)["value"];
      return fvalue > match;
    } else if (JSON.typeof((*condition)["value_input"]) == "number") {
      int input_num = (*condition)["value_input"];
      Input input = inputs[input_num-1];
      str = input_get(&input,true);
      double match = str.toDouble();
      return fvalue > match;
    } else if (JSON.typeof((*condition)["value_output"]) == "number") {
      int output_num = (*condition)["value_output"];
      Output output = outputs[output_num-1];
      double match = 0;
      if (output.type == DOUBLE_VAR) {
        match = preferences.getDouble(output.key.c_str());
      } else if (output.type == FLOAT_VAR) {
        match = preferences.getFloat(output.key.c_str());
      } else {
        str = output_get(&output);
        match = str.toDouble();
      }
      return fvalue > match;
    }
    return false;
  }
  if (type == "night") {
    struct tm timeinfo;
    if (!wifi_time_read(&timeinfo)) {
      return true;
    }
    if (JSON.typeof((*condition)["alba"]) != "number") {
      return false;
    }
    if (JSON.typeof((*condition)["tramonto"]) != "number") {
      return false;
    }
    int alba_num = (*condition)["alba"];
    int tramonto_num = (*condition)["tramonto"];
    String alba = output_get(&outputs[alba_num-1]);
    String tramonto = output_get(&outputs[tramonto_num-1]);
    char time_str[6];
    strftime(time_str,sizeof(time_str),"%H:%M",&timeinfo);
    log_d("time %s alba %s tramonto %s",time_str,alba,tramonto);
    return alba.compareTo(time_str) > 0 || tramonto.compareTo(time_str) < 0;
  }
  log_d("rule %d condition invalid",rule->num);
  return false;
}

void rules_action_run(Rule* rule,JSONVar* action,bool condition,TriggerSpec* spec,int value,double fvalue) {
  if (JSON.typeof((*action)["type"]) != "string") {
    log_d("rule %d action dont have type",rule->num);
    return;
  }
  String type = (*action)["type"];
  if (type == "alert") {
    bool alert_status = (rule->context)["alert_status"];
    if (alert_status == condition) {
      //log_d("rule %d alert_status skip",rule->num);
      return;
    }
    if (JSON.typeof((*action)["alert_num"]) != "number") {
      //log_d("rule %d alert_num skip",rule->num);
      return;
    }
    uint8_t alert_num = (*action)["alert_num"];
    if (condition) {
      //log_d("rule %d alert_on",rule->num);
      if (JSON.typeof((*action)["alert_on"]) == "string") {
        String alert_value = (*action)["alert_on"];
        output_write(&outputs[alert_num-1],alert_value);
      }
    } else {
      //log_d("rule %d alert_off",rule->num);
      if (JSON.typeof((*action)["alert_off"]) == "string") {
        String alert_value = (*action)["alert_off"];
        output_write(&outputs[alert_num-1],alert_value);
      }
    }
    (rule->context)["alert_status"] = condition;
    return;
  }
  if (type == "publish_analog_switch") {
    bool condition_prev = (rule->context)["condition_prev"];
    if (JSON.typeof((rule->context)["condition_prev"]) != "boolean" || condition != condition_prev) {
      if (JSON.typeof((*action)["input"]) == "number") {
        int input_num = (*action)["input"];
        item_publish((inputs[input_num-1].name).c_str(),value);
        (rule->context)["condition_prev"] = condition;
      }
    }
    return;
  }
  log_d("rule %d action invalid",rule->num);
}

void rules_run(Rule* rule,TriggerSpec* spec,int value,double fvalue) {
  log_d("rules_run %d",rule->num);
  bool condition_value = true;
  if (JSON.typeof((rule->config)["condition"]) == "object") {
    JSONVar condition = (rule->config)["condition"];
    condition_value = rules_condition_eval(rule,&condition,spec,value,fvalue);
  } else if (JSON.typeof((rule->config)["conditions"]) == "array") {
    JSONVar conditions = (rule->config)["conditions"];
    for (int j = 0; j < conditions.length(); j++) {
      JSONVar condition = conditions[j];
      condition_value &= rules_condition_eval(rule,&condition,spec,value,fvalue);
      if (!condition_value) {
        break;
      }
    }
  } else {
    log_d("rule dont have conditions");
    return;
  }
  log_d("rules_run %d condition %d",rule->num,condition_value);
  if (JSON.typeof((rule->config)["action"]) == "object") {
    JSONVar action = (rule->config)["action"];
    rules_action_run(rule,&action,condition_value,spec,value,fvalue);
  } else if (JSON.typeof((rule->config)["actions"]) == "array") {
    JSONVar actions = (rule->config)["actions"];
    for (int j = 0; j < actions.length(); j++) {
      JSONVar action = actions[j];
      rules_action_run(rule,&action,condition_value,spec,value,fvalue);
    }
  } else {
    log_d("rule dont have actions");
  }
}

void rules_trigger_setup(Rule* rule,int trigger_num,JSONVar* trigger) {
  if (triggers_size == CONF_RULES_TRIGGERS_SIZE) {
    log_d("triggers_size reached");
    return;
  }
  if (JSON.typeof((*trigger)["type"]) != "string") {
    log_d("trigger %d dont have type",trigger_num);
    return;
  }
  String type = (*trigger)["type"];
  if (type == "pin" || type == "button") {
    if (JSON.typeof((*trigger)["pin"]) != "number") {
      log_d("trigger %d dont have pin",trigger_num);
      return;
    }
    int pin = (*trigger)["pin"];
    triggers[triggers_size].rule = rule;
    triggers[triggers_size].spec.source_type = TRIGGER_PIN;
    triggers[triggers_size].spec.source_num = pin;
    if (type == "button") {
      triggers[triggers_size].spec.input_type = DIGITAL_INPUT;
      triggers[triggers_size].spec.change = true;
    } else {
      triggers[triggers_size].spec.input_type = NO_INPUT;
      triggers[triggers_size].spec.change = false;
    }
    triggers[triggers_size].handler = [](Rule* rule,TriggerSpec* spec,int value,double fvalue,input_type_t itype,bool change) {
      if ((spec->input_type == NO_INPUT || spec->input_type == itype) && (!spec->change || change)) {
        rules_run(rule,spec,value,fvalue);
      }
    };
    pin_triggers[pin][triggers_size] = true;
    triggers_size++;
    return;
  }
  if (type == "output") {
    if (JSON.typeof((*trigger)["num"]) != "number") {
      log_d("trigger %d dont have num",trigger_num);
      return;
    }
    int num = (*trigger)["num"];
    triggers[triggers_size].rule = rule;
    triggers[triggers_size].spec.source_type = TRIGGER_OUTPUT;
    triggers[triggers_size].spec.source_num = num;
    triggers[triggers_size].spec.input_type = NO_INPUT;
    triggers[triggers_size].spec.change = false;
    triggers[triggers_size].handler = [](Rule* rule,TriggerSpec* spec,int value,double fvalue,input_type_t itype,bool change) {
      rules_run(rule,spec,value,fvalue);
    };
    output_triggers[num-1][triggers_size] = true;
    triggers_size++;
    return;
  }
  log_d("trigger %d invalid type %s",trigger_num,type);
}

void rules_setup(String str) {
  for (uint8_t i = 0; i != CONF_SCHEMA_PIN_COUNT; i++) {
    for (uint8_t j = 0; j != CONF_RULES_TRIGGERS_SIZE; j++) {
      pin_triggers[i][j] = false;
    }
  }
  for (uint8_t i = 0; i != CONF_SCHEMA_OUTPUT_COUNT; i++) {
    for (uint8_t j = 0; j != CONF_RULES_TRIGGERS_SIZE; j++) {
      output_triggers[i][j] = false;
    }
  }
  JSONVar json_array = JSON.parse(str);
  if (JSON.typeof(json_array) != "array") {
    log_d("rules isnt array");
    return;
  }
  log_d("rules setup");
  for (int i = 0; i < json_array.length(); i++) {
    if (rules_size == CONF_RULES_TRIGGERS_SIZE) {
      log_d("rules_size reached");
      return;
    }
    JSONVar json = json_array[i];
    rules[rules_size].num = rules_size+1;
    rules[rules_size].config = json;
    if (JSON.typeof(json["trigger"]) == "object") {
      JSONVar trigger = json["trigger"];
      rules_trigger_setup(&rules[rules_size],0,&trigger);
    } else if (JSON.typeof(json["triggers"]) == "array") {
      JSONVar triggers = json["triggers"];
      for (int j = 0; j < triggers.length(); j++) {
        JSONVar trigger = triggers[j];
        rules_trigger_setup(&rules[rules_size],j,&trigger);
      }
    } else {
      log_d("rule %d dont have triggers",i);
      continue;
    }
    rules_size++;
  }
}

#endif

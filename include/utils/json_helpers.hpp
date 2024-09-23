#pragma once
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include "nlohmann/json.hpp"

#define JSON_MAX_LONG_LONG (0x1000000000000LL)

namespace neuro 
{

inline static bool json_boolean(const nlohmann::json &p, const std::string &key, bool default_if_not_there)
{
  if (p.contains(key)) return p[key];
  return default_if_not_there;
}

inline static std::string json_string(const nlohmann::json &p, 
                               const std::string &key, 
                               const std::string &default_if_not_there)
{
  if (p.contains(key)) return p[key];
  return default_if_not_there;
}

inline static double json_double(const nlohmann::json &p, 
                          const std::string &key, 
                          double default_if_not_there)
{
  if (p.contains(key)) return p[key];
  return default_if_not_there;
}

inline static long long json_long_long(const nlohmann::json &p, 
                          const std::string &key, 
                          long long default_if_not_there)
{
  if (p.contains(key)) return p[key];
  return default_if_not_there;
}

inline static nlohmann::json json_json(const nlohmann::json &p, 
                          const std::string &key, 
                          const nlohmann::json &default_if_not_there)
{
  if (p.contains(key)) return p[key];
  return default_if_not_there;
}

/* Parameter_Check_Json_T works like Parameter_Check_Json, but throws an
   error instead of always returning a string -- it throws std::runtime_error().

   Specs should be a json object that has key/value pairs that are string/string.
   The keys are legal json parameters and vals are:

      "B" - Boolean
      "L" - Number (Json doesn't differentiate)
      "I" - Number (Json doesn't differentiate)
      "D" - Number (Json doesn't differentiate)
      "C" - Char
      "S" - String
      "J" - JSON object
      "A" - JSON array
      "U" - Unspecified.

  Optionally, you can have a key "Necessary", whose val is a JSON array of strings.

  Parameter_Check_Json() will:

  - Go through params, and check that each key is in specs, and of the correct time.
  - If a key is not in params, it is an error.
  - If params doesn't have a "Necessary" key, it will return an error.

  It returns an error string.  If the string is empty, then all is fine.
 */

inline static void Parameter_Check_Json_T (const nlohmann::json &params, const nlohmann::json &specs)
{
  nlohmann::json::const_iterator pit;
  size_t i;
  std::ostringstream oss;
  std::string val;
  std::string estring;
  bool problem;
  
  oss.str("");

  if (params.size() > 0 && !params.is_object()) {
    throw std::runtime_error("JSON is not an object.\n");
  }

  if (specs.contains("Necessary") && !specs["Necessary"].is_array()) {
    throw std::runtime_error("Parameter_Check_Json: specs['Necessary'] needs to be a JSON array.");
  }

  for (pit = params.begin(); pit != params.end(); pit++) {
    // std::cout << pit.key() << std::endl; /* HERE */
    if (!specs.contains(pit.key())) {
      oss << "Illegal parameter: " << pit.key() << std::endl;
      throw std::runtime_error(oss.str());
    } else if (!specs[pit.key()].is_string()) {
      oss << "Bad json for Json_Parameter_Check() - val needs to be a string: " 
          << pit.key() << std::endl;
      estring = oss.str();
      throw std::runtime_error(estring);
    } else {
      val = specs[pit.key()];
      try {
        switch (val[0]) {
          case 'B': if (!pit->is_boolean()) throw "must be a boolean."; break;
          case 'L': 
          case 'D': 
          case 'C': 
          case 'I': if (!pit->is_number()) throw "must be a number."; break;
          case 'S': if (!pit->is_string()) throw "must be a string."; break;
          case 'J': if (!pit->is_object()) throw "must be a json object."; break;
          case 'A': if (!pit->is_array()) throw "must be a json array."; break;
          case 'U': break;
          default: 
            fprintf(stderr, "Parameter_Check_Json_T: Unknown key '%c'\n", val[0]);
            exit(1);
        }
      } catch (const char *s) {
        oss << "Parameter " << pit.key() << ": " << s << std::endl;
        estring = oss.str();
        throw std::runtime_error(estring);
      }
    }
  }
    
  problem = false;

  if (specs.contains("Necessary")) {
    for (i = 0; i < specs["Necessary"].size(); i++) {
      if (!params.is_object() || !params.contains(specs["Necessary"][i])) {
        oss << "Missing parameter " << specs["Necessary"][i] << std::endl;
        problem = true;
      }
    }
  }

  if (problem) {
    estring = oss.str();
    throw std::runtime_error(estring);
  }
}

inline static std::string Parameter_Check_Json (const nlohmann::json &params, 
                                                const nlohmann::json &specs)
{
  try {
    Parameter_Check_Json_T(params, specs);
  } catch (const std::runtime_error &e) {
    return (std::string) e.what();
  }
  return "";
}

inline static std::string Parameter_Check (const nlohmann::json &p, 
                                    const std::map <std::string, char> &legal, 
                                    const std::vector <std::string> &necessary) 
{
  nlohmann::json::const_iterator pit;
  nlohmann::json specs;
  std::map <std::string, char>::const_iterator lit;
  size_t i; 
  std::string v;

  for (lit = legal.begin(); lit != legal.end(); lit++) {
    v.clear();
    v.push_back(lit->second);
    specs[lit->first] = v;
  }
  
  if (necessary.size() > 0) {
    specs["Necessary"] = nlohmann::json::array();
    for (i = 0; i < necessary.size(); i++) {
      specs["Necessary"].push_back(necessary[i]);
    }
  }  
  return Parameter_Check_Json(p, specs);
}

inline static nlohmann::json json_from_string_or_file(const std::string &s)
{
  nlohmann::json j;
  std::ifstream f;
  bool fopen;
  std::string es;
  
  fopen = false;
  try {
    if (s.find('{') != std::string::npos || s.find('[') != std::string::npos
                                         || s.find('"') != std::string::npos) {
      j = nlohmann::json::parse(s);
    } else if (s.find('.') != std::string::npos) {
      f.open(s.c_str());
      if (f.fail()) throw std::runtime_error((std::string) "Couldn't open " + s);
      fopen = true;
      f >> j;
      f.close();
    } else {
      j = s;
    }
  } catch (const nlohmann::json::exception &e) {
    es = e.what();
    if (fopen) f.close();
    throw std::runtime_error(es);
  }
  return j;
}


inline static std::string pretty_json_helper(const nlohmann::json &j, 
                                             int indent_space = 2,
                                             int indent = 2,
                                             size_t print_width = 150,
                                             bool indent_bracket = true) {
  size_t i;
  std::string rv, dump;
  char buf[1024];
  nlohmann::json::const_iterator jit;
  rv = "";

  /*
    Three cases - 
      1. bool/number/string - we simply return it.
      2. if the object/array string length + some indent < print_widith. we simply return it.
      3. recursively traverse array or object.

    If the value is followed by ":", we don't want to indent again.
  */


  // case 1
  if (j.is_boolean() || j.is_number() || j.is_string()) return j.dump();

  // case 2
  dump = j.dump();
  if (dump.size() + indent - indent_space < print_width) {
    if (indent_bracket) rv += std::string(indent - indent_space, ' ');
    rv += dump;
    return rv;
  } 

  // case 3
  if (j.is_object()) {
  
    if (indent_bracket) snprintf(buf, 1024, "%*s{\n", indent - indent_space, "");
    else snprintf(buf, 1024, "{\n");
    rv += buf;

    for (i = 0, jit = j.begin(); jit != j.end(); ++jit, i++) {
      rv += std::string(indent, ' ') + "\"" + jit.key() + "\": " + pretty_json_helper(jit.value(), indent_space, indent + indent_space, print_width, false);
      if (i != j.size() - 1) rv += ",";
      rv += "\n";
    }
    snprintf(buf, 1024, "%*s}", indent - indent_space, "");
    rv += buf;
     
  } else if (j.is_array()) {
    
    if (indent_bracket) snprintf(buf, 1024, "%*s[\n", indent - indent_space, "");
    else snprintf(buf, 1024, "[\n");
    rv += buf;
    for (i = 0, jit = j.begin(); jit != j.end(); ++jit, i++) {

      if (!jit.value().is_object() && !jit.value().is_array()) rv += std::string(indent, ' ');
      rv += pretty_json_helper(jit.value(), indent_space, indent + indent_space, print_width, true);
      
      if (i != j.size() - 1) rv += ",";
      rv += "\n";
    }
    snprintf(buf, 1024, "%*s]", indent - indent_space, "");
    rv += buf;
  }

  return rv;

}

inline static std::string pretty_json(const nlohmann::json &j, int indent = 2, size_t print_width = 150) {
  return pretty_json_helper(j, indent, indent, print_width, true);
}





};

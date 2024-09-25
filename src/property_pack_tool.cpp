#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include "framework.hpp"

using namespace std;
using namespace neuro;
using nlohmann::json;

void print_commands(FILE *f)
{
  fprintf(f, "For commands that take a json either put a filename on the same line,\n");
  fprintf(f, "or the json can be multiple lines, starting on the next line.\n");
  fprintf(f, "\n");

  fprintf(f, "FJ json                 - Read a PropertyPack from  json.\n");
  fprintf(f, "TJ [file]               - Create JSON from the property pack.\n");
  fprintf(f, "C                       - Clear.\n");
  fprintf(f, "N name type min max cnt - Add a node property (add_node_property).\n");
  fprintf(f, "E name type min max cnt - Add a edge property (add_edge_property).\n");
  fprintf(f, "W name type min max cnt - Add a network property (add_network_property).\n");
  fprintf(f, "EQ json                 - Is the current PropertyPack equal to the json?\n");
  fprintf(f, "NEQ json                - Is the current PropertyPack not equal to the json?\n");
  fprintf(f, "\n");
  fprintf(f, "?                       - Print commands.\n");
  fprintf(f, "Q                       - Quit.\n");
}

bool read_json(const vector <string> &sv, size_t starting_field, json &rv)
{
  bool success;
  string s;
  ifstream fin;

  rv.clear();
  if (starting_field < sv.size()) {
    fin.clear();
    fin.open(sv[starting_field].c_str());
    if (fin.fail()) { 
      perror(sv[starting_field].c_str());
      return false;
    } 
    try { fin >> rv; success = true; } catch(...) { success = false; }
    fin.close();
    return success;
      
  } else {
    try {
      cin >> rv;
      getline(cin, s);
      return true;
    } catch (...) {
      return false;
    }
  }
}

int main(int argc, char **argv)
{
  ofstream fout;
  istringstream ss;
  vector <string> sv;
  string s, l;
  string prompt;
  json j1, j2;
  PropertyPack *pp;
  Property::Type pt;
  int index, count;
  double min, max;
  PropertyPack pp2;

  pp = new PropertyPack;

  if (argc > 2 || (argc == 2 && strcmp(argv[1], "--help") == 0)) {
    fprintf(stderr, "usage: property_pack_tool [prompt]\n");
    fprintf(stderr, "\n");
    print_commands(stderr);
    exit(1);
  }

  if (argc == 2) {
    prompt = argv[1];
    prompt += " ";
  }

  while (1) {
    try {
      if (prompt != "") printf("%s", prompt.c_str());
      if (!getline(cin, l)) return 0;
      sv.clear();
      ss.clear();
      ss.str(l);
      while (ss >> s) sv.push_back(s);
  
      /* Basic commands. */
  
      if (sv.size() == 0) {
      } else if (sv[0][0] == '#') {
      } else if (sv[0] == "?") {
        print_commands(stdout);
      } else if (sv[0] == "Q") {
        exit(0);
  
      /* Test the equality overload */
  
      } else if (sv[0] == "EQ" || sv[0] == "NEQ") {
  
        try {
          if (!read_json(sv, 1, j1)) throw std::runtime_error("Bad Json");
          pp2.from_json(j1);
          if (*pp == pp2) {
            printf("%s\n", (sv[0][0] == 'E') ? "Yes" : "No");
          } else {
            printf("%s\n", (sv[0][0] == 'N') ? "Yes" : "No");
          }
        } catch (const std::runtime_error &e) {
          printf("%s\n", e.what());
        }
  
      /* Reading/Writing JSON */
  
      } else if (sv[0] == "FJ") {
  
        try {
          if (!read_json(sv, 1, j1)) throw std::runtime_error("Bad Json");
          pp->from_json(j1);
        } catch (const std::runtime_error &e) {
          printf("%s\n", e.what());
        }
  
      } else if (sv[0] == "TJ") {
        if (sv.size() > 2) throw runtime_error("usage: TJ [file]\n");
        if (sv.size() == 1) {
          cout << pp->pretty_json() << endl;
        } else {
          fout.clear();
          fout.open(sv[1].c_str());
          if (fout.fail()) throw runtime_error((string) "Couldn't open " + sv[1]);
          fout << pp->as_json().dump() << endl;
          fout.close();
        }
  
      } else if (sv[0] == "C") {
        pp->clear();

      /* Add properties. */

      } else if (sv[0] == "N" || sv[0] == "E" || sv[0] == "W") {

        if (sv.size() != 6) {
          throw runtime_error((string) "usage: " + sv[0] + " name type min max cnt");
        }
        if (sv[2].size() != 1 || ((string) "IDB").find(sv[2][0]) == string::npos) {
          throw runtime_error("bad type");
        }
        switch (sv[2][0]) {
          case 'B': pt = Property::Type::BOOLEAN; break;
          case 'I': pt = Property::Type::INTEGER; break;
          case 'D': pt = Property::Type::DOUBLE; break;
          default: throw std::logic_error("Switch");
        }
        if (sscanf(sv[3].c_str(), "%lf", &min) == 0) throw runtime_error("bad min");
        if (sscanf(sv[4].c_str(), "%lf", &max) == 0) throw runtime_error("bad max");
        if (sscanf(sv[5].c_str(), "%d", &count) == 0) throw runtime_error("bad count");

        try {
          switch(sv[0][0]) {
            case 'N': index = pp->add_node_property(sv[1], min, max, pt, count); break;
            case 'E': index = pp->add_edge_property(sv[1], min, max, pt, count); break;
            case 'W': index = pp->add_network_property(sv[1], min, max, pt, count); break;
            default: throw std::logic_error("Switch");
          }
          printf("Added: index = %d\n", index);
        } catch (const std::runtime_error &e) {
          printf("%s\n", e.what());
        }


      } else {
        throw runtime_error((string) "Invalid command " + sv[0].c_str() + 
                                     ". Use '?' to print a list of commands");
      }
    } catch (const std::runtime_error &e) {
      printf("%s\n", e.what());
    }
  }
  return 0;
}



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

  fprintf(f, "FJ json            - Read an instance of the property class from json.\n");
  fprintf(f, "TJ [file]          - Create JSON from the property.\n");
  fprintf(f, "C n i s t min max  - Create an instance from name index size type min max.\n");
  fprintf(f, "STUFF              - Test the copy constructor, move constructor, assignment.\n");
  fprintf(f, "\n");
  fprintf(f, "?                  - Print commands.\n");
  fprintf(f, "Q                  - Quit.\n");
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

/* The procedure call should test the copy constructor.
   The variable declaration also tests the copy constructor, maybe?
   Returning will test the move constructor.
 */

Property get_from_procedure(Property p)
{
  Property p2 = p;
  return p2;
}

int main(int argc, char **argv)
{
  int i;
  vector <Property> test_vector;
  ofstream fout;
  istringstream ss;
  vector <string> sv;
  string s, l;
  string prompt;
  json j1, j2;
  Property *p;
  Property::Type pt;
  int index, size;
  double min, max;

  p = NULL;

  if (argc > 2 || (argc == 2 && strcmp(argv[1], "--help") == 0)) {
    fprintf(stderr, "usage: property_tool [prompt]\n");
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
  
      /* Reading/Writing JSON */
  
      } else if (sv[0] == "FJ") {
  
        try {
          if (!read_json(sv, 1, j1)) throw("Bad Json");
          if (p != NULL) delete p;
          p = new Property(j1);
        } catch (std::runtime_error e) {
          printf("%s\n", e.what());
          p = NULL;
        }
  
      } else if (sv[0] == "TJ") {
        if (p == NULL) throw runtime_error("No property defined yet");
        if (sv.size() > 2) throw runtime_error("usage: TJ [file]\n");
        if (sv.size() == 1) {
          cout << p->pretty_json() << endl;
        } else {
          fout.clear();
          fout.open(sv[1].c_str());
          if (fout.fail()) throw runtime_error((string) "Couldn't open " + sv[1]);
          fout << p->as_json().dump() << endl;
          fout.close();
        }
  
      /* Create from params */

      } else if (sv[0] == "C") {
  
        if (sv.size() != 7) throw runtime_error("usage: C n i s t min max\n");
        if (sscanf(sv[2].c_str(), "%d", &index) == 0) throw runtime_error("bad index");
        if (sscanf(sv[3].c_str(), "%d", &size) == 0) throw runtime_error("bad size");
        if (sv[4].size() != 1 || ((string) "IDB").find(sv[4][0]) == string::npos) {
          throw runtime_error("bad type");
        }
        switch (sv[4][0]) {
          case 'B': pt = Property::Type::BOOLEAN; break;
          case 'I': pt = Property::Type::INTEGER; break;
          case 'D': pt = Property::Type::DOUBLE; break;
          default: throw std::logic_error("Switch");
        }
        if (sscanf(sv[5].c_str(), "%lf", &min) == 0) throw runtime_error("bad min");
        if (sscanf(sv[6].c_str(), "%lf", &max) == 0) throw runtime_error("bad max");
        if (p != NULL) delete p;
        
        try {
          p = new Property(sv[1], index, size, min, max, pt);
        } catch (std::runtime_error e) {
          printf("%s\n", e.what());
          p = NULL;
        }

      } else if (sv[0] == "STUFF") {
        if (p == NULL) throw runtime_error("No property defined yet");
        Property p2 = get_from_procedure(*p);
        if (p2 == *p) {
          printf("Passed Test 1\n");
          p2.max_value++;
          if (p2 != *p) {
            printf("Passed Test 2\n");
          } else {
            printf("Failed Test 2\n");
          }
        } else {
          printf("Failed Test 2\n");
        }
        test_vector.clear();
        test_vector.resize(10, *p);
        test_vector.resize(20, p2);
        test_vector.resize(30, *p);
        for (i = 0; i < 100; i++) test_vector.push_back(test_vector[0]);
        for (i = 0; i < (int) test_vector.size(); i++) {
          if (i < 10 || i >= 20) {
            if (test_vector[i] != *p) {
              printf("Test 3 failed at index %d\n", i);
            }
          } else {
            if (test_vector[i] != p2) {
              printf("Test 3 failed at index %d\n", i);
            }
          }
        }
        printf("Test 3 Done (If there's no other output, that means it passed.\n");
  
      } else {
        throw runtime_error((string) "Invalid command " + sv[0].c_str() + 
                                     ". Use '?' to print a list of commands");
      }
    } catch (std::runtime_error e) {
      printf("%s\n", e.what());
    }
  }
  return 0;
}



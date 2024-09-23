#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <unistd.h>
#include "framework.hpp"
#include "utils/json_helpers.hpp"
#include "utils/sys_helpers.hpp"

using namespace std;
using namespace neuro;
using nlohmann::json;

typedef runtime_error SRE;

void print_commands(FILE *f)
{
  fprintf(f, "This is a network tool program. The commands listed below are case-insensitive,\n");
  fprintf(f, "For commands that take a json either put a filename on the same line,\n");
  fprintf(f, "or the json can be multiple lines, starting on the next line.\n");

  fprintf(f, "\nCreate/Clear Network Commands\n");
  fprintf(f, "FJ json                    - Read a network.\n");
  fprintf(f, "TJ [file]                  - Create JSON from the network.\n");
  fprintf(f, "COPY_FROM                  - Make a copy of yourself using copy_from.  Print & delete.\n");
  fprintf(f, "DESTROY                    - Delete network, create empty network.\n");
  fprintf(f, "CLEAR                      - Clear network\n");
  fprintf(f, "CLEAR_KP                   - Clear network but keep the property pack intact\n");     
  
  fprintf(f, "\nAccess Network Info Commands\n");
  fprintf(f, "INFO                       - Print some info about the network.\n");
  fprintf(f, "NODES [node_id] [...]      - Print the nodes using a kind of user-friendly JSON format.\n");
  fprintf(f, "EDGES [from] [to] [...]    - Print the edges using a kind of user-friendly JSON format.\n");
  fprintf(f, "PROPERTIES/P               - Print the network's property pack.\n");
  fprintf(f, "ASSOC_DATA [key]           - Print the network's associated_data, or just that key.\n");
  fprintf(f, "TYPE node_id  ...          - Print the node's type\n");
  fprintf(f, "NM                         - Print node names map\n");
  
  fprintf(f, "\nNetwork Operation Commands\n");

  fprintf(f, "PRUNE                      - Prune the network - remove nodes/edges not on an I/O path.\n");
  fprintf(f, "AN node_id ...             - Add nodes\n");
  fprintf(f, "AI node_id ...             - Add inputs\n");
  fprintf(f, "AO node_id ...             - Add outputs\n");
  fprintf(f, "AE from to ...             - Add edges \n");
  fprintf(f, "RN node_id ... [IOE(T|F)]  - Remove nodes. IOE (def:F) flags an error if Input/Output\n");
  fprintf(f, "RE from to ...             - Remove edges\n");
  fprintf(f, "RENAME from to             - Rename nodes\n");
  fprintf(f, "SETNAME node_id name       - Set the names of nodes. A name of \"-\" clears the name\n");
  fprintf(f, "SETCOORDS node_id [x] [y]  - Set the coordinates of a node. This is used by viz\n");
  fprintf(f, "SET_CP node_id [x] [y]     - Set the control points of a node. This is used by viz\n");
  fprintf(f, "CLEAR_VIZ                  - Get rid of all coordinates and control points in the network\n");

  fprintf(f, "SNP node_id ... name value - Set the node's named property to value.\n");
  fprintf(f, "SEP from to ... name value - Set the edge's named property to value.\n");
  fprintf(f, "SNP_ALL name value         - Set named property to value for all nodes.\n");
  fprintf(f, "SEP_ALL name value         - Set named property to value for all edges.\n");
  fprintf(f, "RNP node_id ... [pname]    - Randomize the node's properties or named property\n");
  fprintf(f, "REP from to ... [pname]    - Randomize the edge's properties or named property\n");

  fprintf(f, "SEED val                   - Seed the RNG\n");
  fprintf(f, "SHOW_SEED                  - Show the RNG seed\n");
  fprintf(f, "RE_SEED                    - Re-seed the RNG based on the current time & show seed\n");
  fprintf(f, "SPROPERTIES/SP json        - Set the network's property pack\n");
  fprintf(f, "SET_ASSOC key json         - Set the key/val in the network's associated data.\n");
  fprintf(f, "SORT/SORTED [Q]            - Sort the network and print the sorted node id's. Q = no output\n");
 
  fprintf(f, "\nHelper Commands\n");
  fprintf(f, "RUN                        - Run the app.\n");
  fprintf(f, "?                          - Print commands.\n");
  fprintf(f, "Q                          - Quit.\n");
}


bool is_number(const string& str) {
  size_t i;

  for (i = 0; i < str.size(); i++) {
    if (std::isdigit(str[i]) == 0) return false;
  }

  return true;
}

void to_uppercase(string &s) 
{
  size_t i;
  for (i = 0; i < s.size(); i++) {
    if (s[i] >= 'a' && s[i] <= 'z') {
      s[i] = s[i] + 'A' -'a';
    }
  }
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

int get_node_id_by_name(const string &name, map <string, uint32_t> &node_names) {
  if (node_names.find(name) != node_names.end()) return (int) node_names[name];
  return -1;
}

string get_node_name(Node *n) 
{
  string id;
  
  id = std::to_string(n->id);
  if (n->name == "") return id;
  return id + "(" + n->name + ")";
}


int main(int argc, char **argv)
{
  Network *n, *n2;
  Edge *e;
  Node *node;
  PropertyPack pp;
  const Property *prop;
  NodeMap::iterator nit;
  EdgeMap::iterator eit;

  istringstream ss;
  ofstream fout;
  vector <string> sv;
  vector <string> keys;
  vector <double> dv;
  map <string, uint32_t> node_names, tmp_map;
  map <string, uint32_t>::const_iterator it;
  string s, l, app_name, proc_name;
  string prompt, cmd;
  json j1;
  int lowest_free_id;
  uint32_t seed;

  double d;
  int from, to, id;
  size_t i,size;

  MOA rng;

  d = 0;

  if (argc > 2 || (argc == 2 && strcmp(argv[1], "--help") == 0)) {
    fprintf(stderr, "usage: network_tool [prompt]\n");
    fprintf(stderr, "\n");
    print_commands(stderr);
    exit(1);
  }

  seed = rng.Seed_From_Time();
  rng.Seed(seed, "network_tool");

  if (argc == 2) {
    prompt = argv[1];
    prompt += " ";
  }

  n = new Network;
  n2 = new Network;
  e = NULL;
  lowest_free_id = 0;

  while (1) {
    if (prompt != "") printf("%s", prompt.c_str());
    if (!getline(cin, l)) return 0;
    sv.clear();
    ss.clear();
    ss.str(l);
    while (ss >> s) sv.push_back(s);

    /* Basic commands. */
    size = sv.size();
    if(size != 0) to_uppercase(sv[0]); 

    if (size == 0) {
    } else if (sv[0][0] == '#') {
    } else if (sv[0] == "?") {
      print_commands(stdout);
    } else if (sv[0] == "Q") {
      exit(0);

    /* To / From Json. */

    } else if (sv[0] == "FJ") {
      if (!read_json(sv, 1, j1)) {
        printf("Bad json.\n");
      } else {
        try {
          node_names.clear();
          lowest_free_id = 0;

          n->from_json(j1);

          for (nit = n->begin(); nit != n->end(); nit++) {
            node = nit->second.get();
            node_names[std::to_string(node->id)] = node->id;
          }

        } catch (SRE &e) {
          printf("%s\n", e.what());
        }
      }
    } else if (sv[0] == "SEED") {
      if (sv.size() != 2 || sscanf(sv[1].c_str(), "%u", &seed) != 1) {
        printf("usage: SEED val\n");
      } else {
        rng.Seed(seed, "network_tool");
      }

    } else if (sv[0] == "SHOW_SEED") {
      printf("%u\n", seed);

    } else if (sv[0] == "RE_SEED") {
      seed = rng.Seed_From_Time();
      rng.Seed(seed, "network_tool");
      printf("%u\n", seed);

    } else if (sv[0] == "TJ") {
      try {
        if (size > 1) {
          fout.clear();
          fout.open(sv[1].c_str());
          if (fout.fail()) throw SRE((string) "Couldn't open " + sv[1]);
          fout << n->pretty_json() << endl;
          fout.close();
        } else {
          cout << n->pretty_json() << endl;
        }
      } catch (SRE &e) {
        printf("%s\n", e.what());
      }

    } else if (sv[0] == "PRUNE") {
      try {
        if (size != 1) throw SRE("usage: PRUNE");
        n->prune();

        /* Clear pruned nodes out of the name map. */

        tmp_map.clear();
        for (it = node_names.begin(); it != node_names.end(); ++it) {
          if (n->is_node(it->second)) tmp_map[it->first] = it->second;
        }

        node_names = tmp_map;
        lowest_free_id = 0;
          
      } catch (SRE &e) {
        printf("%s\n", e.what());
      }

    } else if (sv[0] == "COPY_FROM") {
      *n2 = *n;
      j1 = json::object();
      n2->to_json(j1);
      cout << j1.dump(2) << endl;

    } else if (sv[0] == "DESTROY") {
      delete n;
      n = new Network;
      node_names.clear();
      lowest_free_id = 0;

    } else if (sv[0] == "NODES") {

      if (size == 1) {
        cout << n->pretty_nodes() << endl;
      } else {
        for (i = 1; i < size; i++) {
          try {
            id = get_node_id_by_name(sv[i], node_names);
            if (id == -1) throw SRE(sv[i] + " is not a valid node");
            node = n->get_node(id);
            j1 = { {"id", node->id}, 
                   {"values", node->values} };
            if (node->coordinates.size() != 0) j1["coords"] = j1["coords"] = node->coordinates;
            cout << j1.dump() << endl;
          } catch (SRE &e) {
            printf("%s\n",e.what());
          } 
        }
      }

    } else if (sv[0] == "EDGES") {

      if(size == 1) {
        cout << n->pretty_edges() << endl;
      } else if ((size - 1) % 2 != 0) {
        printf("usage: EDGES [from1] [to1] [from2] [to2] ...\n");
      } else {
        for (i = 0; i < (size - 1) / 2; i++) {
          try {
            from = get_node_id_by_name(sv[1+i*2], node_names);
            to = get_node_id_by_name(sv[2+i*2], node_names);
            if (from == -1 || to == -1) {
              throw SRE(sv[1+i*2] + " -> " + sv[2+i*2] + " is not a valid edge");
            } 
            e = n->get_edge(from, to);
            cout << e->as_json() << endl;

          } catch (SRE &e) {
            printf("%s\n",e.what());
          }
        }
      }

    } else if (sv[0] == "SORTED" || sv[0] == "SORT") {
      if (sv.size() > 2 || (sv.size() == 2 && sv[1] != "Q")) {
        printf("usage: SORT/SORTED [Q] - Sort the network and print the sorted node id's.");
        printf(" Q = no output\n");
      }
      n->make_sorted_node_vector();
      if (sv.size() == 1) {
        for (i = 0; i < n->sorted_node_vector.size(); i++) {
          if (i != 0) printf(" ");
          printf("%u", n->sorted_node_vector[i]->id);
        }
        printf("\n");
      }

    } else if (sv[0] == "PROPERTIES" || sv[0] == "P") {
      cout << n->get_properties().pretty_json() << endl;

    } else if (sv[0] == "SPROPERTIES" || sv[0] == "SP") {
      if (!read_json(sv, 1, j1)) {
        printf("SPROPERTIES: Bad json.\n");
      } else {
        try {
          pp.from_json(j1);
          n->set_properties(pp);
        } catch (SRE &e) {
          printf("%s\n", e.what());
        }
      }

    } else if (sv[0] == "INFO") {

      printf("Nodes:   %8d\n", (int) n->num_nodes());
      printf("Edges:   %8d\n", (int) n->num_edges());
      printf("Inputs:  %8d\n", (int) n->num_inputs());
      printf("Outputs: %8d\n", (int) n->num_outputs());

      printf("\n");
      printf("Input nodes:  ");
      for (i = 0; i < (size_t)n->num_inputs(); i++) {
        node = n->get_input(i);
        printf("%s ", get_node_name(node).c_str());
      }
      printf("\n");

      printf("Hidden nodes: ");
      for (nit = n->begin(); nit != n->end(); nit++) {
        node = nit->second.get();
        if (node->is_hidden()) {
          printf("%s ", get_node_name(node).c_str());
        }
      }
      printf("\n");

      printf("Output nodes: ");
      for (i = 0; i < (size_t)n->num_outputs(); i++) {
        node = n->get_output(i);
        printf("%s ", get_node_name(node).c_str());
      }
      printf("\n");

    } else if (sv[0] == "CLEAR-KP" || sv[0] == "CLEAR_KP") {   // Keeping the - for backward compatability.
      n->clear(false);
      node_names.clear();
      lowest_free_id = 0;

    } else if (sv[0] == "CLEAR") {
      n->clear(true);
      node_names.clear();
      lowest_free_id = 0;

    } else if (sv[0] == "NM") {
      if (size == 1) cout << (json) node_names << endl;
      else {
        for (i = 1; i < size; i++ ) {
          try {
            if (get_node_id_by_name(sv[i], node_names) == -1) {
              throw SRE((string) "Node " + sv[i] + " does not exist");
            } else {
              printf("\"%s\": %d\n", sv[i].c_str(), node_names[sv[i]]);
            }
          } catch (SRE &e) {
            printf("%s\n",e.what());
          } 
            
        }
      }
    } else if (sv[0] == "AE") {  // add_edge()

      if ((size - 1) % 2 != 0 || size < 3) {
        printf("usage: AE from1 to1 from2 to2 ...\n");
      } else {

        for (i = 0; i < (size - 1) / 2; i++) {
          try {
            from = get_node_id_by_name(sv[1+i*2], node_names);
            to = get_node_id_by_name(sv[2+i*2], node_names);
            if (from == -1 || to == -1) {
              throw SRE(sv[1+i*2] + " -> " + sv[2+i*2] + " is not a valid edge");
            }
            
            n->add_edge(from, to);

          } catch (SRE &e) {
            printf("%s\n",e.what());
          } 
            
        }
      }

    } else if (sv[0] == "SEP") { // set edge property

      if (size < 5 || (size - 3) % 2 != 0) {
        printf("usage: SEP from1 to1 from2 to2 ... name value\n");

      } else if (sscanf(sv[size - 1].c_str(), "%lf", &d) != 1) {
        printf("%s is not a valid property value.\n", sv[size - 1].c_str());

      } else if (!n->is_edge_property(sv[size - 2])) {
        printf("edge property \"%s\" doesn't exist.\n", sv[size - 2].c_str());

      } else {
          
        for (i = 0; i < (size - 3) / 2; i++) {
          try {
            from = get_node_id_by_name(sv[1+i*2], node_names);
            to = get_node_id_by_name(sv[2+i*2], node_names);
            if (from == -1 || to == -1) {
              throw SRE(sv[1+i*2] + " -> " + sv[2+i*2] + " is not a valid edge");
            }
            e = n->get_edge(from, to);
            e->set(sv[size - 2], d);
          } catch (SRE &e) {
            printf("%s\n",e.what());
          }

        }
      }

    } else if (sv[0] == "SEP_ALL") {  // set property for all edges

      if (size != 3) {
        printf("usage: SEP_ALL name value\n");
      } else if (sscanf(sv[size - 1].c_str(), "%lf", &d) != 1) {
        printf("%s is not a valid property value.\n", sv[size - 1].c_str());

      } else if (!n->is_edge_property(sv[size - 2])) {
        printf("edge property \"%s\" doesn't exist.\n", sv[size - 2].c_str());

      } else {
        prop = n->get_edge_property(sv[size-2]);
        for (eit = n->edges_begin(); eit != n->edges_end(); eit++) {
          e = eit->second.get();
          e->set(prop->index, d);
        }
      }

    } else if (sv[0] == "SNP") {  // set node property

      if (size < 4) {
        printf("usage: SNP node_id1 node_id2 ... name value\n");
      } else if (sscanf(sv[size - 1].c_str(), "%lf", &d) != 1) {
        printf("%s is not a valid property value.\n", sv[size - 1].c_str());

      } else if (!n->is_node_property(sv[size - 2])) {
        printf("node property \"%s\" doesn't exist.\n", sv[size - 2].c_str());

      } else {
        for (i = 1; i < size - 2; i++) {
          try {
            id = get_node_id_by_name(sv[i], node_names);
            if (id == -1) throw SRE(sv[i] + " is not a valid node");
            
            node = n->get_node(id);
            node->set(sv[size - 2], d);

          } catch (SRE &e) {
            printf("%s\n",e.what());
          }

        }
      }

    } else if (sv[0] == "SNP_ALL") {  // set property for all nodes

      if (size != 3) {
        printf("usage: SNP_ALL name value\n");
      } else if (sscanf(sv[size - 1].c_str(), "%lf", &d) != 1) {
        printf("%s is not a valid property value.\n", sv[size - 1].c_str());

      } else if (!n->is_node_property(sv[size - 2])) {
        printf("node property \"%s\" doesn't exist.\n", sv[size - 2].c_str());

      } else {
        prop = n->get_node_property(sv[size-2]);
        for (nit = n->begin(); nit != n->end(); nit++) {
          node = nit->second.get();
          node->set(prop->index, d);
        }
      }

    } else if (sv[0] == "AN") { // add_node()
      if (size < 2) {
        printf("usage: AN node_id1 node_id2 ...\n");
      } else {

        for (i = 1; i < size; i++ ) {
          try {
            if (get_node_id_by_name(sv[i], node_names) != -1) {
              throw SRE((string) "Node " + sv[i] + " already exists");
            }
            
            if (is_number(sv[i])) {
              id = stoi(sv[i]);
              if (n->is_node(id)) throw SRE((string) "Node " + sv[i] + " already exists");
              n->add_node(id);
              node_names[sv[i]] = id;

            } else {
              while (n->is_node(lowest_free_id)) lowest_free_id++;
              node_names[sv[i]] = lowest_free_id;
              node = n->add_node(lowest_free_id);
              node->name = sv[i];
              lowest_free_id++;
            }

          } catch (SRE &e) {
            printf("%s\n",e.what());
          } 
            
        }
         
      }
      
    } else if (sv[0] == "AI") { // add_input()
      if (size < 2) {
        printf("usage: AI node_id1 node_id2 ...\n");
      } else {

        for (i = 1; i < size; i++ ) {
          try {
            id = get_node_id_by_name(sv[i], node_names);
            if (id == -1) throw SRE(sv[i] + " is not a valid node");
            n->add_input(id);
          } catch (SRE &e) {
            printf("%s\n",e.what());
          } 
            
        }
      }

    } else if (sv[0] == "AO") { // add_output()
      if (size < 2) {
        printf("usage: AO node_id1 node_id2\n");
      } else {

        for (i = 1; i < size; i++ ) {
          try {
            id = get_node_id_by_name(sv[i], node_names);
            if (id == -1) throw SRE(sv[i] + " is not a valid node");
            n->add_output(id);
          } catch (SRE &e) {
            printf("%s\n",e.what());
          } 
            
        }
      }

    } else if (sv[0] == "RN") { // remove_node()
      if (size < 2) {
        printf("usage: RN node_id1 node_id2 ... [T|F]\n");
      } else {

        if (sv[size - 1] != "T" && sv[size - 1] != "F") {
          sv.push_back("F");
          size++;
        }

        for (i = 1; i < size - 1; i++ ) {
          try {
            id = get_node_id_by_name(sv[i], node_names);
            if (id == -1) throw SRE(sv[i] + " is not a valid node");
            n->remove_node(id, sv[size - 1] == "F");

          } catch (SRE &e) {
            printf("%s\n",e.what());
          } 
            
        }
      }


    } else if (sv[0] == "RE") { // remove_edge()
      if (size < 3 || (size - 1) % 2 != 0) {
        printf("usage: RE from1 to1 from2 to2 ...\n");
      } else {

        for (i = 0; i < (size - 1) / 2; i++) {
          try {

            from = get_node_id_by_name(sv[1+i*2], node_names);
            to = get_node_id_by_name(sv[2+i*2], node_names);
            if (from == -1 || to == -1) {
              throw SRE(sv[1+i*2] + " -> " + sv[2+i*2] + " is not a valid edge");
            }
            n->remove_edge(from, to);
          } catch (SRE &e) {
            printf("%s\n",e.what());
          } 
            
        }
      }

    } else if (sv[0] == "RENAME") { // rename_node()

      if (size != 3) {
        printf("usage: RENAME from to\n");
      } else {
        try {
          from = get_node_id_by_name(sv[1], node_names);
          to = get_node_id_by_name(sv[2], node_names);
          if (from == -1 || to == -1) {
            throw SRE(sv[1] + " -> " + sv[2] + " is not a valid edge");
          }
          n->rename_node(from, to);
        } catch (SRE &e) {
          printf("%s\n",e.what());
        }
        
      }
    } else if (sv[0] == "RNP") { // randomize node property.

      if (size < 2) {
        printf("usage: RNP node_id1 node_id2 ... [pname]\n");
      } else if (get_node_id_by_name(sv[size - 1], node_names) == -1) { // pname is specified
        if (!n->is_node_property(sv[size - 1])) {
          printf("There is neither a node nor a property named \"%s\"\n", sv[size - 1].c_str());
        } else {
          for (i = 1; i < size - 1; i++) {
            try {
              id = get_node_id_by_name(sv[i], node_names);
              if (id == -1) throw SRE(sv[i] + " is not a valid node");
              node = n->get_node(id);
              n->randomize_property(rng, node, sv[size - 1]);

            } catch (SRE &e) {
              printf("%s\n",e.what());
            }
          } 
        }
      } else { // pname is not specified. We randomize all properties of node
        for (i = 1; i < size; i++) {
          try {
            id = get_node_id_by_name(sv[i], node_names);
            if (id == -1) throw SRE(sv[i] + " is not a valid node");
            node = n->get_node(id);
            n->randomize_properties(rng, node);

          } catch (SRE &e) {
            printf("%s\n",e.what());
          }
        } 
      }

    } else if (sv[0] == "REP") { // randomize edge property
      
      if (size < 3) {
        printf("usage: REP from1 to1 from2 to2 ... [pname]\n");
      } else if (get_node_id_by_name(sv[size - 1], node_names) == -1) { // pname is specified
        if (!n->is_edge_property(sv[size - 1])) {
          printf("edge property \"%s\" doesn't exist.\n", sv[size - 1].c_str());
        } else if ((size - 2) % 2 != 0) {
          printf("usage: REP from1 to1 from2 to2 ... [pname]\n");
        } else {
          for (i = 0; i < (size - 2) / 2; i++) {

            try {
              from = get_node_id_by_name(sv[1+i*2], node_names);
              to = get_node_id_by_name(sv[2+i*2], node_names);
              if (from == -1 || to == -1) {
                throw SRE(sv[1+i*2] + " -> " + sv[2+i*2] + " is not a valid edge");
              }

              e = n->get_edge(from, to);
              n->randomize_property(rng, e, sv[size - 1]);

            } catch (SRE &e) {
              printf("%s\n",e.what());
            }
          } 
        }
      } else { // pname is not specified. We randomize all properties of edge
        if ((size - 1) % 2 != 0) {
          printf("usage: REP from1 to1 from2 to2 ... [pname]\n");
        } else {
          for (i = 0; i < (size - 1) / 2; i++) {
            try {
              from = get_node_id_by_name(sv[1+i*2], node_names);
              to = get_node_id_by_name(sv[2+i*2], node_names);
              if (from == -1 || to == -1) {
                throw SRE(sv[1+i*2] + " -> " + sv[2+i*2] + " is not a valid edge");
              }
              

              e = n->get_edge(from, to);
              n->randomize_properties(rng, e);

            } catch (SRE &e) {
              printf("%s\n",e.what());
            }
          } 
        }
      }

    } else if (sv[0] == "TYPE") { // print the node type
      if (size < 2) {
        printf("usage: TYPE node_id1 node_id2 ...\n");
      } else {

        for (i = 1; i < size; i++) {
          try {
            id = get_node_id_by_name(sv[i], node_names);
            if (id == -1) throw SRE(sv[i] + " is not a valid node");
            
            node = n->get_node(id);
            if (node->is_input() && node->is_output()) {
              printf("%d is an input and output node\n", id);
            } else if (node->is_input()) {
              printf("%d is an input node\n", id);
            } else if (node->is_output()) {
              printf("%d is an output node\n", id);
            } else {
              printf("%d is a hidden node\n", id);
            }

          } catch (SRE &e) {
            printf("%s\n",e.what());
          } 
        }
      }

    } else if (sv[0] == "ASSOC_DATA") {
      if (sv.size() > 2) {
        printf("usage: ASSOC_DATA [key] - Print the network's associated_data, or just that key.\n");
      } else if (sv.size() == 1) {
        keys = n->data_keys();
        for (i = 0; i < keys.size(); i++) {
          j1 = n->get_data(keys[i]);
          printf(" Key %s : %s\n", keys[i].c_str(), pretty_json_helper(j1).c_str());
        }
      } else {
        j1 = n->get_data(sv[1]);
        cout << j1 << endl;
      }

    } else if (sv[0] == "SET_ASSOC") {
      if (sv.size() < 2) {
        printf("Usage: SET_ASSOC key json - json is filename, or json that starts on the next line.\n");
      } else if (read_json(sv, 2, j1)) {
        n->set_data(sv[1], j1);
      }
      
    } else if (sv[0] == "SETNAME") {
      if (sv.size() != 3) {
        printf("Usage: SETNAME node_id name - Set the names of nodes. A name of \"-\" clears the name\n");
      } else {

        try {
          id = get_node_id_by_name(sv[1], node_names);
          if (id == -1) throw SRE(sv[1] + " is not a valid node");
          node = n->get_node(id);

          if (sv[2] == "-") node->name = "";
          else node->name = sv[2];
        } catch (SRE &e) {
          printf("%s\n",e.what());
        } 

      }
    } else if (sv[0] == "SETCOORDS") {

      if (sv.size() <= 1) {
        printf("Usage: SETCOORDS node_id [x] [y] - Set the coordinates of a node. This is used by viz\n");
      } else {

        try {
          id = get_node_id_by_name(sv[1], node_names);
          if (id == -1) throw SRE(sv[1] + " is not a valid node");
          node = n->get_node(id);

          dv.clear();
          for (i = 2; i < std::min( (size_t) 4, sv.size()); i++) dv.push_back(stod(sv[i]));
          node->coordinates = dv;

        } catch (SRE &e) {
          printf("%s\n",e.what());
        } catch (std::invalid_argument const& e) {
          printf("%s\n",e.what());
        }
      }

    } else if (sv[0] == "CLEAR_VIZ") {
      for (nit = n->begin(); nit != n->end(); nit++) {
        node = nit->second.get();
        node->coordinates.clear();
        for (i = 0; i < node->outgoing.size(); i++) node->outgoing[i]->control_point.clear();
      }

    } else if (sv[0] == "SET_CP") {

      if (sv.size() <= 2) {
        printf("Usage: SET_CP from to [x] [y] - Set the control points of a edge. This is used by viz\n");
      } else {

        try {
          from = get_node_id_by_name(sv[1], node_names);
          to = get_node_id_by_name(sv[2], node_names);

          if (from == -1 || to == -1) {
            throw SRE(sv[1] + " -> " + sv[2] + " is not a valid edge");
          }
          e = n->get_edge(from, to);
          dv.clear();
          for (i = 3; i < std::min( (size_t) 5, sv.size()); i++) dv.push_back(stod(sv[i]));
          e->control_point = dv;
        } catch (SRE &e) {
          printf("%s\n",e.what());
        } 
      }


    } else if (sv[0] == "RUN") {
      j1 = n->as_json();
      
      if (!j1["Associated_Data"]["other"].contains("app_name")) {
        printf("app name is not specified in the network\n");
      } else if (!j1["Associated_Data"]["other"].contains("app_name")) {
        printf("proc name is not specified in the network\n");
      } else {
        app_name = j1["Associated_Data"]["other"]["app_name"];
        proc_name = j1["Associated_Data"]["other"]["proc_name"];
        cmd = "cpp-apps/bin/" + app_name + "_" + proc_name + " -a test -n NETWORK_TOOL_NETWORK_TEMP.JSON";

        fout.clear();
        fout.open("NETWORK_TOOL_NETWORK_TEMP.JSON");
        if (fout.fail()) printf("Couldn't write to NETWORK_TOOL_NETWORK_TEMP.JSON file for app to run\n"); 
        else {
          fout << j1 << endl;
          fout.close();
          printf("running %s_%s\n", app_name.c_str(), proc_name.c_str());
          if(system(cmd.c_str()) == -1) {
            printf("Fail to run app. Check if the cpp-apps/bin/%s_%s exists\n", app_name.c_str(), proc_name.c_str());
          }
          i = system("rm -f NETWORK_TOOL_NETWORK_TEMP.JSON");
        }

      }
     
    } else {
      printf("Invalid command %s.  Use '?' to print a list of commands.\n", sv[0].c_str());
    }

  }
 
  return 0;
}

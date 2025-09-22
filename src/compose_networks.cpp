#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <unistd.h>
#include "framework.hpp"

using namespace std;
using namespace neuro;
using nlohmann::json;

typedef runtime_error SRE;

json json_from_string_or_file(const string &s)
{
  json j;
  ifstream f;
  bool fopen;
  string es;
  
  fopen = false;               
  try {                        
    if (s.find('{') != string::npos || s.find('[') != string::npos
                                         || s.find('"') != string::npos) {
      j = json::parse(s);
    } else if (s.find('.') != string::npos) {
      f.open(s.c_str());
      if (f.fail()) throw runtime_error((string) "Couldn't open " + s);
      fopen = true;
      f >> j;
      f.close();
    } else {
      j = s;
    }
  } catch (const json::exception &e) {
    es = e.what();
    if (fopen) f.close();
    throw runtime_error(es);
  }
  return j;
}

int main(int argc, char **argv)
{
  size_t i, j;
  uint32_t id, f, t;
  json n1_json, n2_json;
  Network n1_network, n2_network;
  map <uint32_t, Node *> n1_nodes, n2_nodes, n2_new;
  map <uint32_t, Node *>::iterator nmit;
  Edge *e, *ne;
  Node *n, *nn, *n1, *n2;
  NodeMap::iterator nit;
  EdgeMap::iterator eit;
  char buf[200];
  istringstream ss;
  string l, w;
  double d;
  vector <double> vd;
  vector < vector <double> > conflate;
  vector < vector <double> > synapse;
  string tag;

  try {

    if (argc != 4) throw SRE("usage: compose_networks n1 n2 n2-tag\n");
    tag = argv[3];

    while (getline(cin, l)) {
      ss.clear();
      ss.str(l);
      if (ss >> w) {
        if (w == "CONFLATE" || w == "SYNAPSE") {
          vd.clear();
          while (ss >> d) vd.push_back(d);
          if (w == "CONFLATE") {
            if (vd.size() != 2) throw SRE(l + "\nCONFLATE should have n1 and n2");
            conflate.push_back(vd);
          }
          if (w == "SYNAPSE") {
            if (vd.size() <= 3) throw SRE(l + "\nSYNAPSE should have n1 and n2 and then each prop");
            synapse.push_back(vd);
          }
        } else {
          throw SRE(l + "\nUnknown command.  Should be CONFLATE or SYNAPSE");
        }
      }
    }
    /* Read in the two networks */

    n1_json = json_from_string_or_file(argv[1]);
    n2_json = json_from_string_or_file(argv[2]);
    n1_network.from_json(n1_json);
    n2_network.from_json(n2_json);

    /* Create the two node maps */

    for (nit = n1_network.begin(); nit != n1_network.end(); nit++) {
      n = nit->second.get();
      n1_nodes[n->id] = n;
    }
    for (nit = n2_network.begin(); nit != n2_network.end(); nit++) {
      n = nit->second.get();
      n2_nodes[n->id] = n;
    }

    /* Now, go through the conflation vector, and make sure the nodes exist
       in both networks.  Put the node pointers into n2_new.  */

    for (i = 0; i < conflate.size(); i++) {
      if (n1_nodes.find(conflate[i][0]) == n1_nodes.end()) {
        snprintf(buf, 200, "CONFLATE %d %d\n", (int) conflate[i][0], (int) conflate[i][1]);
        throw SRE((string) buf + "Could not find the first node in the first network");
      }
      n1 = n1_nodes[conflate[i][0]];
      if (n2_nodes.find(conflate[i][1]) == n2_nodes.end()) {
        snprintf(buf, 200, "CONFLATE %d %d\n", (int) conflate[i][0], (int) conflate[i][1]);
        throw SRE((string) buf + "Could not find the second node in the second network");
      }
      n2 = n2_nodes[conflate[i][1]];

      n2_new[n2->id] = n1;
      
      /* If the tag is "", then just keep the first name.  Otherwise, put a 
         slash between the two names. */

      if (tag != "") n1->name += ((string) "/" + tag + n2->name);
    }

    /* Go through all of the nodes in n2, and for those that are not conflated, create
       new nodes in n1 for them.  Put these new nodes into n2_new. */

    id = 0;

    for (nmit = n2_nodes.begin(); nmit != n2_nodes.end(); nmit++) {
      n = nmit->second;
      if (n2_new.find(n->id) == n2_new.end()) {
        while (n1_network.is_node(id)) id++;
        nn = n1_network.add_node(id);
        n2_new[n->id] = nn;
        nn->values = n->values;
        nn->name = tag + n->name;
        nn->coordinates = n->coordinates;
      }
    }
      
    /* Go through the edges in n2, and add them to n1. */

    for (eit = n2_network.edges_begin(); eit != n2_network.edges_end(); eit++) {
      e = eit->second.get();
      n1 = e->from;
      n2 = e->to;
      if (n2_new.find(n1->id) == n2_new.end()) throw SRE ("compose_networks I error 0");
      if (n2_new.find(n2->id) == n2_new.end()) throw SRE ("compose_networks I error 1");

      ne = n1_network.add_edge(n2_new[n1->id]->id, n2_new[n2->id]->id);
      if (ne == NULL) {
        snprintf(buf, 200, "Error creating edge in n1.  %d -> %d already exists.",
              n2_new[n1->id]->id, n2_new[n2->id]->id);
        throw SRE(buf);
      }
      ne->values = e->values;
      ne->control_point = e->control_point;
    }
    
    /* Create the new edges */

    for (i = 0; i < synapse.size(); i++) {
      f = synapse[i][0];
      t = synapse[i][1];
      if (!n1_network.is_node(f)) {
        snprintf(buf, 200, "SYNAPSE Error: Node %d does not exist in the first network.", f);
        throw SRE(buf);
      }
      if (n2_new.find(t) == n2_new.end()) {
        snprintf(buf, 200, "SYNAPSE Error: Node %d does not exist in the second network.", t);
        throw SRE(buf);
      }
      ne = n1_network.add_edge(f, n2_new[t]->id);
      if (ne == NULL) {
        snprintf(buf, 200, "Error creating edge in n1.  %d -> %d already exists.",
                f, n2_new[t]->id);
        throw SRE(buf);
      }
      for (j = 2; j < synapse[i].size(); j++) ne->values[j-2] = synapse[i][j];
    
    }

    /* Finally, add outputs from the second network to the first network, and
       add inputs from the second network to the first network.  */

    for (i = 0; i < (size_t) n2_network.num_outputs(); i++) {
      n = n2_network.get_output(i);
      nn = n2_new[n->id];
      if (nn->output_id == -1) n1_network.add_output(nn->id);
    }

    for (i = 0; i < (size_t) n2_network.num_inputs(); i++) {
      n = n2_network.get_input(i);
      nn = n2_new[n->id];
      if (nn->input_id == -1) n1_network.add_input(nn->id);
    }

    n1_network.make_sorted_node_vector();
    cout << n1_network.pretty_json() << endl;
    
  } catch (SRE &e) {
    fprintf(stderr, "%s\n", e.what());
    exit(1);
  }
 
  return 0;
}

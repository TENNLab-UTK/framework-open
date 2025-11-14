#include <vector>
#include <unordered_map>
#include <unordered_set>
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

class Conflation {
  public:
    string name;
    vector <int> networks;
    vector <Node *> nodes;
    Node *n;                 // This is the node in the new graph.
};

class Synapse_Spec {
  public:
    int net_from;
    int net_to;
    Node *neuron_from;
    Node *neuron_to;
    vector <int> indices;
    vector <double> values;
};

class IO_Spec {
  public:
    int network;
    Node *neuron;
    char type;
};

class Composition {
  public:
    void read_conflation(const vector <string> &sv, const string &line);
    void read_tag(const vector <string> &sv, const string &line);
    void read_synapse(const vector <string> &sv, const string &line);
    void read_and_add_neuron(const vector <string> &sv, const string &line);
    void read_input_output(const vector <string> &sv, const string &line, char type);
    void read_networks(int argc, char **argv);
    void set_up_rv();
    void copy_nodes();
    void copy_edges();
    void copy_network_values();
    void apply_synapses();
    void add_io();

    bool input_specified;
    bool output_specified;
    vector <json> n_json;
    vector <Network> nets;
    vector < unordered_map <uint32_t, Node *> > nodemaps;
    vector <PropertyPack> packs;
    vector <string> tags;
    uint32_t nodes;
    Network rv;
    PropertyPack pp;
    vector <Conflation *> conflations;
    vector <Synapse_Spec *> synapses;
    vector <IO_Spec *> ios;
    unordered_map <string, Conflation *> node_to_conflation;
};

void Composition::read_input_output(const vector <string> &sv, const string &line, char type)
{
  IO_Spec *io;
  size_t i;
  int netnum;
  uint32_t nodeid;
  string tmp;
  
  if (type == 'I') input_specified = true;
  if (type == 'O') output_specified = true;

  try {
    if (sv.size() %2 != 0) {
      throw (string) "usage: INPUT|OUTPUT [ network_id ALL|neuron ] ...";
    }
    for (i = 0; i < sv.size(); i += 2) {
      if (sscanf(sv[i].c_str(), "%d", &netnum) != 1 || netnum < 0 || netnum >= (int) nets.size()) {
        throw (string) "Bad network number " + sv[i];
      }
      if (sv[i+1] != "ALL") {
        if (sscanf(sv[i+1].c_str(), "%u", &nodeid) != 1 || !nets[netnum].is_node(nodeid)) {
          throw (string) "Bad node (" + sv[i+1] + ") in network " + sv[i];
        }
      }
      io = new IO_Spec;
      io->network = netnum;
      io->type = type;
      if (sv[i+1] == "ALL") {
        io->neuron = NULL;
      } else {
        io->neuron = nets[netnum].get_node(nodeid);
      }
      ios.push_back(io);
    }

  } catch (const string &s) {
    tmp = "Error reading INPUT|OUTPUT:\n" + line + "\n" + s;
    throw SRE(tmp);
  }
}

void Composition::apply_synapses()
{
  size_t i, j;
  Synapse_Spec *s;
  Edge *e;
  Node *from, *to;

  /* This has all been error checked already when we read the SYNAPSE line. 
     Note, we use add_or_get_edge() to create the edge if it does not already exist. */

  for (i = 0; i < synapses.size(); i++) {
    s = synapses[i];
    from = nodemaps[s->net_from][s->neuron_from->id];
    to = nodemaps[s->net_to][s->neuron_to->id];
    e = rv.add_or_get_edge(from->id, to->id);
    for (j = 0; j < s->indices.size(); j++) e->values[s->indices[j]] = s->values[j];
  }
}

void Composition::add_io()
{
  IO_Spec *io;
  size_t i;
  int j;
  Node *n;
  vector <string> sv;
  char buf[100];

  if (!input_specified) {
    sv.clear();
    for (i = 0; i < nets.size(); i++) {
      snprintf(buf, 100, "%d", (int) i);
      sv.push_back(buf);
      sv.push_back("ALL");
    }
    read_input_output(sv, "Inside add_io()", 'I');
  }

  if (!output_specified) {
    sv.clear();
    for (i = 0; i < nets.size(); i++) {
      snprintf(buf, 100, "%d", (int) i);
      sv.push_back(buf);
      sv.push_back("ALL");
    }
    read_input_output(sv, "Inside add_io()", 'O');
  }

  for (i = 0 ; i < ios.size(); i++) {
    io = ios[i];
    if (io->neuron != NULL) {
      n = nodemaps[io->network][io->neuron->id];
      if (io->type == 'I') {
        if (!n->is_input()) rv.add_input(n->id);
      } else {
        if (!n->is_output()) rv.add_output(n->id);
      }
    } else if (io->type == 'I') {
      for (j = 0; j < nets[io->network].num_inputs(); j++) {
        n = nets[io->network].get_input(j);
        n = nodemaps[io->network][n->id];
        if (!n->is_input()) rv.add_input(n->id);
      }
    } else {
      for (j = 0; j < nets[io->network].num_outputs(); j++) {
        n = nets[io->network].get_output(j);
        n = nodemaps[io->network][n->id];
        if (!n->is_output()) rv.add_output(n->id);
      }
    }
  }
}

void Composition::read_synapse(const vector <string> &sv, const string &line)
{
  size_t i;
  int netnum, netfrom, netto;
  uint32_t nodeid;
  Node *from, *to;
  vector <int> property_indices;
  vector <double> values;
  double v;
  Synapse_Spec *s;
  string tmp;
  PropertyMap::iterator pit;

  try {
    if (sv.size() %2 != 0 || sv.size() < 4) {
      throw (string) "usage: SYNAPSE from-network from-neuron to-network to-neuron " + 
                     "[ property-name property-value ] ...";
    }
    for (i = 0; i < 4; i += 2) {
      if (sscanf(sv[i].c_str(), "%d", &netnum) != 1 || netnum < 0 || netnum >= (int) nets.size()) {
        throw (string) "Bad network number " + sv[i];
      }
      if (sscanf(sv[i+1].c_str(), "%u", &nodeid) != 1 || !nets[netnum].is_node(nodeid)) {
        throw (string) "Bad node (" + sv[i+1] + ") in network " + sv[i];
      }
      if (i == 0) { from = nets[netnum].get_node(nodeid); netfrom = netnum; }
      if (i == 2) { to = nets[netnum].get_node(nodeid); netto = netnum; }
    }
 
    for (i = 4; i < sv.size(); i += 2) {
      if (pp.edges.find(sv[i]) == pp.edges.end()) throw (string) "Bad property name: " + sv[i];
      if (sscanf(sv[i+1].c_str(), "%lf", &v) == 0) {
        throw (string) "Bad property value: " + sv[i+1];
      }
      pit = pp.edges.find(sv[i]);
      property_indices.push_back(pit->second.index);
      values.push_back(v);
    }

    s = new Synapse_Spec;
    s->net_from = netfrom;
    s->net_to = netto;
    s->neuron_from = from;
    s->neuron_to = to;
    s->indices = property_indices;
    s->values = values;
    synapses.push_back(s);
    
  } catch (const string &s) {
    tmp = "Error reading SYNAPSE:\n" + line + "\n" + s;
    throw SRE(tmp);
  }
}

void Composition::read_and_add_neuron(const vector <string> &sv, const string &line)
{
  size_t i, j;
  int netnum;
  uint32_t nodeid;
  vector <int> property_indices;
  vector <double> values;
  double v;
  Node *nn;
  string tmp;
  PropertyMap::iterator pit;
  string name;

  try {
    if (sv.size() %2 != 0 || sv.size() < 2) {
      throw (string) "usage: NEURON network id [ NAME name ] " + 
                     "[ property-name property-value ] ...";
    }
    if (sscanf(sv[0].c_str(), "%d", &netnum) != 1 || netnum < 0 || netnum >= (int) nets.size()) {
      throw (string) "Bad network number " + sv[0];
    }
    if (sscanf(sv[1].c_str(), "%u", &nodeid) != 1) {
      throw (string) "Bad node (" + sv[1] + ") in network " + sv[0];
    }
 
    for (i = 2; i < sv.size(); i += 2) {
      if (sv[i] == "NAME") {
        name = sv[i+1];
      } else {
        if (packs[netnum].nodes.find(sv[i]) == packs[netnum].nodes.end()) {
          throw (string) "Bad property name: " + sv[i];
        }
        if (sscanf(sv[i+1].c_str(), "%lf", &v) == 0) {
          throw (string) "Bad property value: " + sv[i+1];
        }
        pit = packs[netnum].nodes.find(sv[i]);
        property_indices.push_back(pit->second.index);
        values.push_back(v);
      }
    }

    nn = nets[netnum].add_or_get_node(nodeid);
    if (name != "") nn->name = name;
    for (j = 0; j < property_indices.size(); j++) nn->values[property_indices[j]] = values[j];

  } catch (const string &s) {
    tmp = "Error reading NEURON:\n" + line + "\n" + s;
    throw SRE(tmp);
  }
}

void Composition::read_tag(const vector <string> &sv, const string &line)
{
  size_t i;
  int netnum;
  string tmp;

  try {
    if (sv.size() % 2 != 0) throw (string) "# of words on the line should be a multiple of two.";
    for (i = 0; i < sv.size(); i += 2) {
      if (sscanf(sv[i].c_str(), "%d", &netnum) != 1 || netnum < 0 ||
                                                       netnum >= (int) nets.size()) {
        throw (string) "Bad network number " + sv[i];
      }
      if (tags[netnum] != "") throw (string) "More than one tag for network " + sv[i];
      tags[netnum] = sv[i+1];
    }
  } catch (const string &s) {
    tmp = "Error reading TAG:\n" + line + "\n" + s;
    throw SRE(tmp);
  }
}

void Composition::read_conflation(const vector <string> &sv, const string &line)
{
  string tmp;
  size_t i;
  int netnum;
  uint32_t nodeid;
  vector <int> networks;
  vector <uint32_t> node_ids;
  string name;
  Conflation *c;
  char buf[50];
  unordered_set <int> network_ids;

  try {
    if (sv.size() % 2 != 0) throw (string) "# of words on the line should be a multiple of two.";
    for (i = 0; i < sv.size(); i += 2) {
      if (sv[i] == "NAME") {
        if (name != "") throw (string) "Multiple NAME specifications";
        name = sv[i+1];
      } else {
        if (sscanf(sv[i].c_str(), "%d", &netnum) != 1 || netnum < 0 ||  
                                                         netnum >= (int) nets.size()) {
          throw (string) "Bad network number " + sv[i];
        }
        if (network_ids.find(netnum) != network_ids.end()) {
          throw (string) "Cannot conflate nodes in the same network: " + sv[i];
        }
        network_ids.insert(netnum);
        networks.push_back(netnum);
        if (sscanf(sv[i+1].c_str(), "%u", &nodeid) != 1 || !nets[netnum].is_node(nodeid)) {
          throw (string) "Bad node (" + sv[i+1] + ") in network " + sv[i];
        }
        node_ids.push_back(nodeid);
      }
    }
    if (networks.size() < 2) throw (string) "Need to specify more than one node.";

    c = new Conflation;
    c->n = NULL;
    c->name = name;
    c->networks = networks;
    for (i = 0; i < node_ids.size(); i++) {
      c->nodes.push_back(nets[networks[i]].get_node(node_ids[i]));
    }
    conflations.push_back(c);
    for (i = 0; i < node_ids.size(); i++) {
      snprintf(buf, 50, "%d:%u", networks[i], node_ids[i]);
      if (node_to_conflation.find(buf) != node_to_conflation.end()) {
        throw (string) "Cannot specify the same node in more than one conflation: " + buf;
      }
      node_to_conflation[buf] = c;
    }
  } catch (const string &s) {
    tmp = "Error reading CONFLATE:\n" + line + "\n" + s;
    throw SRE(tmp);
  }
}

void Composition::read_networks(int argc, char **argv)
{
  size_t i;

  for (i = 1; (int) i < argc; i++) n_json.push_back(json_from_string_or_file(argv[i]));

  nets.resize(n_json.size());
  packs.resize(n_json.size());
  nodemaps.resize(nets.size());

  for (i = 0; i < n_json.size(); i++) {
    nets[i].from_json(n_json[i]);
  }
  for (i = 0; i < n_json.size(); i++) packs[i] = nets[i].get_properties();
}

void Composition::set_up_rv()
{
  vector <string> associated_keys;
  size_t i;

  rv.clear(true);
  nodes = 0;

  /* Set up the return value network, using property & associated info from the
     first network. */

  pp = packs[0];
  rv.set_properties(pp);
  associated_keys = nets[0].data_keys();
  for (i = 0; i < associated_keys.size(); i++) {
    rv.set_data(associated_keys[i], nets[0].get_data(associated_keys[i]));
  }

  input_specified = false;
  output_specified = false;
  tags.resize(nets.size());
}

void Composition::copy_edges()
{
  Node *nf, *nt;
  Edge *e, *ne;
  EdgeMap::iterator eit;
  size_t i;
  PropertyMap::iterator pit, pit2;
  char buf[200];

  for (i = 0; i < nets.size(); i++) {
    for (eit = nets[i].edges_begin(); eit != nets[i].edges_end(); eit++) {
      e = eit->second.get();
      nf = nodemaps[i][e->from->id];
      nt = nodemaps[i][e->to->id];
      if (!rv.is_edge(nf->id, nt->id)) {
        ne = rv.add_edge(nf->id, nt->id);
        ne->control_point = e->control_point;
        for (pit = packs[i].edges.begin(); pit != packs[i].edges.end(); pit++) {
          pit2 = pp.edges.find(pit->first);
          if (pit2 == pp.edges.end()) {
            snprintf(buf, 200, "Error - Network %d has a edge property %s that's not in rv.\n",
                     (int) i, pit->first.c_str());
            throw SRE((string) buf);
          }
          ne->values[pit2->second.index] = e->values[pit->second.index];
        }
      }
    }
  }
}

void Composition::copy_nodes()
{
  Node *n, *nn;
  size_t i, j;
  PropertyMap::iterator pit, pit2;
  char buf[200];
  Conflation *c;

  for (i = 0; i < nets.size(); i++) {
    for (j = 0; j < nets[i].sorted_node_vector.size(); j++) {
      n = nets[i].sorted_node_vector[j];
      snprintf(buf, 200, "%d:%u", (int) i, n->id);
      nn = NULL;
      if (node_to_conflation.find(buf) != node_to_conflation.end()) {
        c = node_to_conflation[buf];
        if (c->n == NULL) {
          c->n = rv.add_node(nodes);
          nodes++;
          nn = c->n;
          nn->name = c->name;
          nn->coordinates = n->coordinates;
        }
        if (c->name == "") {
          if (c->n->name.size() > 0) c->n->name.push_back('/');
          c->n->name += (tags[i] + n->name);
        }
        nodemaps[i][n->id] = c->n;
      } else {
        nn = rv.add_node(nodes);
        nodemaps[i][n->id] = nn;
        nodes++;
        nn->name = tags[i] + n->name;
        nn->coordinates = n->coordinates;
      }
      if (nn != NULL) {
        for (pit = packs[i].nodes.begin(); pit != packs[i].nodes.end(); pit++) {
          pit2 = pp.nodes.find(pit->first);
          if (pit2 == pp.nodes.end()) {
            snprintf(buf, 200, "Error - Network %d has a node property %s that's not in rv.\n",
                     (int) i, pit->first.c_str());
            throw SRE((string) buf);
          }
          nn->values[pit2->second.index] = n->values[pit->second.index];
        }
      }
    }
  }
}

void Composition::copy_network_values()
{
  NodeMap::iterator nit;
  size_t i;
  int index;
  PropertyMap::iterator pit, pit2;
  char buf[200];

  for (i = 0; i < packs.size(); i++) {
    index = packs.size() - i - 1;
    for (pit = packs[index].networks.begin(); pit != packs[index].networks.end(); pit++) {
      pit2 = pp.networks.find(pit->first);
      if (pit2 == pp.networks.end()) {
        snprintf(buf, 200, "Error - Network %d has a property %s that's not in rv.\n",
                 (int) i, pit->first.c_str());
        throw SRE((string) buf);
      }
      rv.values[pit2->second.index] = nets[index].values[pit->second.index];
    }
  }
}

int main(int argc, char **argv)
{
  Composition C;
  map <uint32_t, uint32_t>::iterator nmit;
  istringstream ss;
  string l, w, s;
  vector <string> sv;
  vector < vector <string> > conflate;
  vector < vector <string> > synapse;
  Network rv;
  size_t i;

  try {

    if (argc < 2) throw SRE("usage: compose_networks n1 n2 ... -- Specs on stdin\n");

    /* Read in the networks */

    C.read_networks(argc, argv);
    C.set_up_rv();

    /* Read in all of the modifiers. */

    while (getline(cin, l)) {
      ss.clear();
      ss.str(l);
      if (ss >> w) {
        sv.clear();
        while (ss >> s) sv.push_back(s);
        if (w == "CONFLATE") {
          C.read_conflation(sv, l);
        } else if (w == "TAG") {
          C.read_tag(sv, l);
        } else if (w == "SYNAPSE") {
          C.read_synapse(sv, l);
        } else if (w == "NEURON") {
          C.read_and_add_neuron(sv, l);
        } else if (w == "OUTPUT") {
          C.read_input_output(sv, l, 'O');
        } else if (w == "INPUT") {
          C.read_input_output(sv, l, 'I');
        } else {
          throw SRE(l + "\nUnknown command.  Should be CONFLATE, SYNAPSE, INPUT or OUTPUT");
        }
      }
    }

    /* It's easier to deal with the sorted_node_vector than it is to use the
       iterators for nodes and edges, so do it here.  Note, you can't do it
       earlier because add_neurons may invalidate it. */

    for (i = 0; i < C.nets.size(); i++) C.nets[i].make_sorted_node_vector();

    /* Process the rest of the modifiers. */

    C.copy_nodes();
    C.copy_edges();
    C.copy_network_values();
    C.apply_synapses();
    C.add_io();

    /* Finish up. */

    C.rv.make_sorted_node_vector();
    cout << C.rv.pretty_json() << endl;

  } catch (SRE &e) {
    fprintf(stderr, "%s\n", e.what());
    exit(1);
  }
 
  return 0;
}

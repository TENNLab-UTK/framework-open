#include "framework.hpp"
#include <cstdio>
#include "robinhood/robin_set.h"
#include "utils/json_helpers.hpp"
#include <cmath>
#include <iostream>

typedef std::runtime_error SRE;

namespace neuro
{

using std::string;
using std::vector;
using std::make_pair;
using nlohmann::json;

static json network_specs = {
   { "Properties", "J" },
   { "Associated_Data", "J" },
   { "Nodes", "A" },
   { "Edges", "A" },
   { "Inputs", "A" },
   { "Outputs", "A" },
   { "Network_Values", "A" },
     { "Necessary", { "Properties", 
                      "Associated_Data", 
                      "Nodes", 
                      "Edges", 
                      "Inputs", 
                      "Outputs", 
                      "Network_Values" } } };

static json node_specs = {
   { "id", "I" },
   { "values", "A" },
   { "name", "S" },
   { "coords", "A" },
     { "Necessary", { "id", "values", } } };

static json edge_specs = {
   { "from", "I" },
   { "to", "I" },
   { "values", "A" },
   { "control_point", "A" },
     { "Necessary", { "from", "to", "values", } } };

static const vector<string> required_edge_keys { "from", "to", "values" };

static bool node_comp(Node *n1, Node *n2) { return (n1->id < n2->id); }

void Node::set(int idx, double val)
{
    values.at(idx) = val;
}

static json node_json(Node *n)
{
  json rv;

  rv = json::object();
  rv["id"] = n->id;
  rv["values"] = (n->values.size() == 0) ? json::array() : (json) n->values;
  if (n->name != "") rv["name"] = n->name;
  if (n->coordinates.size() != 0) rv["coords"] = n->coordinates;
  return rv;
}

/* JSP - want to keep the local variable allocation out of the common path. */

static void tl_ne_set_error(const string &ne, const string &name, double val, const Property &p)
{
  string estring;
  char buf[200];

  snprintf(buf, 200, "%lg", val);
  estring = ne + "::set(" + name + "," + buf + ") - Error: val must be in the range: ";
  snprintf(buf, 200, "[%lg,%lg].", p.min_value, p.max_value);
  estring += buf;
  throw SRE(estring);
}

void Node::set(const string& name, double val)
{
  int idx;

  if(net == nullptr) throw std::runtime_error("No Network pointer is set for this node");
   
  Property p = net->m_properties.nodes.at(name);
  if (val < p.min_value || val > p.max_value) tl_ne_set_error("Node", name, val, p);

  idx = net->m_properties.nodes.at(name).index;
  set(idx, val);
}

double Node::get(int idx)
{
    return values.at(idx);
}

double Node::get(const string& name)
{
    if(net == nullptr) throw std::runtime_error("No Network pointer is set for this node");
    int idx = net->m_properties.nodes.at(name).index;
    return get(idx);
}

json Edge::as_json() const
{
  json rv;

  rv = json::object();
  rv["from"] = from->id;
  rv["to"] = to->id;
  rv["values"] = (values.size() == 0) ? json::array() : (json) values;
  if (control_point.size() != 0) rv["control_point"] = control_point;
  return rv;
}

void Edge::set(int idx, double val)
{
    values.at(idx) = val;
}

void Edge::set(const string& name, double val)
{   
    if(net == nullptr) throw SRE("No Network pointer is set for this edge");

    Property p = net->m_properties.edges.at(name);
    if (val < p.min_value || val > p.max_value) tl_ne_set_error("Edge", name, val, p);

    int idx = net->m_properties.edges.at(name).index;
    set(idx, val);
}

double Edge::get(int idx)
{
    return values.at(idx);
}

double Edge::get(const string& name)
{
    if(net == nullptr) throw std::runtime_error("No Network pointer is set for this edge");
    int idx = net->m_properties.edges.at(name).index;
    return get(idx);
}

Network::Network(const Network& net)
{
    copy_from(net);
}

Network::Network(Network&& net)
{
    move_from(std::move(net));
}

Network& Network::operator=(const Network& net)
{
    copy_from(net);
    return *this;
}

Network& Network::operator=(Network&& net)
{
    move_from(std::move(net));
    return *this;
}

Network::~Network() noexcept
{
    // Nothing needed right now?
}

bool Network::operator==(const Network &rhs) const
{
    // matching node/edge count
    if(num_nodes() != rhs.num_nodes()) return false;
    if(num_edges() != rhs.num_edges()) return false;

    // matching properties
    if (m_properties != rhs.m_properties) return false;

    // matching network values
    if(values != rhs.values) return false;

    // matching inputs / outputs
    if(num_inputs() != rhs.num_inputs()) return false;
    if(num_outputs() != rhs.num_outputs()) return false;
    if(m_inputs != rhs.m_inputs) return false;
    if(m_outputs != rhs.m_outputs) return false;

    // matching nodes
    for(auto &n : m_nodes)
    {
        if(!rhs.is_node(n.first)) return false;

        Node *n1 = n.second.get();
        Node *n2 = rhs.get_node(n.first);

        if(n1->id != n2->id) return false;
        if(n1->input_id != n2->input_id) return false;
        if(n1->output_id != n2->output_id) return false;
    }

    // matching edges
    for(auto &e : m_edges)
    {
        if(!rhs.is_edge(e.first.first, e.first.second)) return false;

        Edge *e1 = e.second.get();
        Edge *e2 = rhs.get_edge(e.first.first, e.first.second);

        if(e1->from->id != e2->from->id) return false;
        if(e1->to->id != e2->to->id) return false;
        if(e1->values != e2->values) return false;
    }

    if (m_associated_data != rhs.m_associated_data) return false;

    return true;
}

void Network::copy_from(const Network& net)
{
    m_properties = net.m_properties;

    // Copy nodes
    for(auto& n : net.m_nodes)
    {
        Node *node = add_node(n.first);
        
        for(size_t i = 0; i < n.second->values.size(); i++)
        {
            node->values[i] = n.second->values[i];
        }
    }

    // Copy edges
    for(auto& e : net.m_edges)
    {
        Edge *edge = add_edge(e.first.first, e.first.second);

        for(size_t i = 0; i < e.second->values.size(); i++)
        {
            edge->values[i] = e.second->values[i];
        }
    }

    // Copy inputs
    for(auto i : net.m_inputs)
        add_input(i);

    // Copy outputs
    for(auto o : net.m_outputs)
        add_output(o);

    m_associated_data = net.m_associated_data;
    values = net.values;
    sorted_node_vector.clear();
}

void Network::move_from(Network&& net)
{
    m_properties = std::move(net.m_properties);

    m_nodes = std::move(net.m_nodes);
    m_edges = std::move(net.m_edges);
    m_inputs = std::move(net.m_inputs);
    m_outputs = std::move(net.m_outputs);
    m_associated_data = std::move(net.m_associated_data);
    values = std::move(net.values);
    sorted_node_vector.clear();
}

void Network::clear(bool include_properties)
{
  if (include_properties) {
    m_properties.clear();
    values.clear();
  }
  m_inputs.clear();
  m_outputs.clear();
  m_nodes.clear();
  m_edges.clear();
  m_associated_data = json::object();
  sorted_node_vector.clear();
}

json Network::as_json() const
{
    auto j = json::object();
    to_json(j);
    return j;
}

void Network::to_json(json& j) const
{
    // Dump all the properties
    // If the properties aren't empty, save them into the JSON

    j["Properties"] = m_properties.as_json();

    j["Nodes"] = json::array();
    for(auto& n : m_nodes)
    {
        j["Nodes"].push_back(node_json(n.second.get()));
    }
    
    // Dump all edges
    j["Edges"] = json::array();
    for(auto& e : m_edges)
    {
        j["Edges"].push_back(e.second.get()->as_json());
    }

    // Inputs & Outputs
    j["Inputs"] = m_inputs;
    j["Outputs"] = m_outputs;
    
    // Dump all network property values

    j["Network_Values"] = (values.size() == 0) ? json::array() : (json) values;

    // Keep all associated data together as a dictionary entry in the network json

    j["Associated_Data"] = (m_associated_data == nullptr) ? json::object() : m_associated_data;
}

void Network::from_json(const json &j)
{
    json tmp_properties;
    Node *n;
    Edge *e;
    string estring;
    size_t i;
    char buf[128];
    uint32_t id;

    /* Check parameters. */

    Parameter_Check_Json_T(j, network_specs);

    clear(true);

    /* Grab the properties. */
   
    m_properties.from_json(j["Properties"]);

    // Add the Network values

    values = j["Network_Values"].get<vector <double>>();
    if (values.size() != m_properties.net_vec_size) {
      throw SRE((string) "Error in network JSON: " +
                "Network_Value's array's size doesn't match the network Propery Pack");
    }

    // Get any associated data

    m_associated_data = json::object();
    if (j["Associated_Data"] != nullptr) m_associated_data = j["Associated_Data"];

    // Add nodes /w values
    // CHZ I didn't pass as reference because we may need to modify jn

    for(auto jn : j["Nodes"])
    {   
        Parameter_Check_Json_T(jn, node_specs);
        n = add_node(jn["id"]);
        n->values = jn["values"].get<vector<double>>();
        if (n->values.size() != m_properties.node_vec_size) {
          estring = "Error in the network JSON: Node " + jn["id"].dump() +
                    "'s value array's size does not match the node PropertyPack";
          throw SRE(estring);
        }
        if (jn.contains("coords")) n->coordinates = jn["coords"].get<vector<double>>();
        if (jn.contains("name")) n->name = jn["name"];
    }

    // Add edges /w values
    for(auto& je : j["Edges"])
    {
        Parameter_Check_Json_T(je, edge_specs);
        e = add_edge(je["from"], je["to"]);
        e->values = je["values"].get<vector<double>>();
        if (e->values.size() != m_properties.edge_vec_size) {
          estring = "Error in the network JSON: Edge " + je["from"].dump() + "->" +
                    je["to"].dump() + 
                    "'s value array's size does not match the edge PropertyPack";
          throw SRE(estring);
        }
        if (je.contains("control_point")) {
          e->control_point = je["control_point"].get<vector<double>>();
        }
    }

    // Set inputs & outputs

    // Add the inputs & outputs

    for (i = 0; i < j["Inputs"].size(); i++) {
      if (j["Inputs"][i].get<double>() < 0) {
        snprintf(buf, 128, "%d", (int) i);
        estring = (string) "Bad Network JSON - Input[" + (string) buf + "] is < 0.";
        throw SRE(estring);
      }
      id = j["Inputs"][i].get<uint32_t>();
      add_input(id);
    }
    
    for (i = 0; i < j["Outputs"].size(); i++) {
      if (j["Outputs"][i].get<double>() < 0) {
        snprintf(buf, 128, "%d", (int) i);
        estring = (string) "Bad Network JSON - Output[" + (string) buf + "] is < 0.";
        throw SRE(estring);
      }
      id = j["Outputs"][i].get<uint32_t>();
      add_output(id);
    }
}

Node* Network::add_node(uint32_t idx)
{
    NodeMap::iterator nit;
    bool inserted;
    PropertyMap::iterator pit;
    int i;
    char buf[200];

    if (is_node(idx)) {
      snprintf(buf, 200, "Node %d already exists at specified index.", idx);
      throw SRE(buf);
    }

    // insert to hash table & move ownership of pointer
    std::tie(nit, inserted) = m_nodes.emplace(idx, make_unique<Node>(idx, this));

    if (!inserted) {
      snprintf(buf, 200, "Could not insert node %u.", idx);
      throw SRE(buf);
    }

    // Any node addition or deletion invalidates the sorted_node_vector.
    sorted_node_vector.clear();

    // resize the values vector to match properties
    nit->second->values.resize(m_properties.node_vec_size);

    // set each value to its max in the property pack
    for (pit = m_properties.nodes.begin(); pit != m_properties.nodes.end(); pit++) {
      for (i = 0; i < pit->second.size; i++) {
        nit->second->values[pit->second.index+i] = pit->second.max_value;
      }
    }

    return nit->second.get();
}

Node* Network::add_or_get_node(uint32_t idx)
{
    if(is_node(idx))
        return get_node(idx);
    else 
        return add_node(idx);
}

Edge* Network::add_edge(uint32_t fr, uint32_t to)
{
    Node *from_node;
    Node *to_node;
    PropertyMap::iterator pit;
    int i;
    char buf[200];

    if (is_edge(fr, to)) {
      snprintf(buf, 200, "Edge %u -> %u already exists.", fr, to);
      throw SRE(buf);
    }

    // get_node will throw if the node does not exist -- this will propagate through this call to prevent the edge
    try {
        from_node = get_node(fr);
        to_node = get_node(to);
    }
    catch(...) {
      snprintf(buf, 200, "Unable to get node %u and/or %u", fr, to);
      throw SRE(buf);
    }

    EdgeMap::iterator eit;
    bool inserted;

    std::tie(eit, inserted) = m_edges.emplace(make_pair(fr, to), make_unique<Edge>(from_node, to_node, this));

    if(!inserted) {
       snprintf(buf, 200, "Could not insert edge %u -> %u", fr, to);
       throw SRE(buf);
    }

    // set the values to their max in the property_pack

    eit->second->values.resize(m_properties.edge_vec_size);
    for (pit = m_properties.edges.begin(); pit != m_properties.edges.end(); pit++) {
      for (i = 0; i < pit->second.size; i++) {
        eit->second->values[pit->second.index+i] = pit->second.max_value;
      }
    }

    from_node->outgoing.push_back(eit->second.get());
    to_node->incoming.push_back(eit->second.get());

    return eit->second.get();
}

Edge* Network::add_or_get_edge(uint32_t fr, uint32_t to)
{
    if(is_edge(fr, to)) 
        return get_edge(fr, to);
    else 
        return add_edge(fr, to);
}
   
void Network::rename_node(uint32_t old_name, uint32_t new_name)
{   
    NodeMap::iterator nit;
    bool inserted;
    Node *n;
    char buf[200];

    if (!is_node(old_name)) {
      snprintf(buf, 200, "Cannot rename a node which does not exist. '%u' -> '%u'.", 
               old_name, new_name);
      throw SRE(buf);
    }

    if(old_name == new_name) return; // nothing to do 

    n = get_node(old_name);
    if (n->is_input() || n->is_output()) {
      snprintf(buf, 200, "Cannot rename an input or output node %u", old_name);
      throw SRE(buf);
    }

    if (is_node(new_name)) {
      snprintf(buf, 200, "Cannot rename a node to a taken name. '%u' -> '%u'.", old_name, new_name);
      throw SRE(buf);
    }

    std::tie(nit, inserted) = m_nodes.emplace(new_name, std::move(m_nodes.at(old_name)));
    
    // Any node addition or deletion invalidates the sorted_node_vector.
    sorted_node_vector.clear();
   
    n = nit->second.get(); // we must get the node before we call erase otherwise we may lose it.
    m_nodes.erase(old_name);
    n->id = new_name;
   
    // Move edges
    for(auto &e : n->incoming)
    {   

        auto fr_idx = e->from->id;
        auto new_edge_name = make_pair(fr_idx, new_name);

        if (fr_idx == new_name) fr_idx = old_name; // self-loop case
        auto old_edge_name = make_pair(fr_idx, old_name);
         
        m_edges.emplace(new_edge_name, std::move(m_edges.at(old_edge_name)));
        m_edges.erase(old_edge_name);
         
       
    }

    for(auto &e : n->outgoing)
    {

        auto to_idx = e->to->id;
        auto old_edge_name = make_pair(old_name, to_idx);
        auto new_edge_name = make_pair(new_name, to_idx);
        if (to_idx != new_name) // self-loop edge only has one instance in m_edges.
        {
            m_edges.emplace(new_edge_name, std::move(m_edges.at(old_edge_name)));
            m_edges.erase(old_edge_name);
        }
    
    }

    
  
}

bool Network::is_node(uint32_t idx) const
{
    return (m_nodes.find(idx) != m_nodes.end());
}

bool Network::is_edge(uint32_t fr, uint32_t to) const
{
    return (m_edges.find(make_pair(fr, to)) != m_edges.end());
}

Node* Network::get_node(uint32_t idx) const
{
    char buf[100];
    auto n = m_nodes.find(idx);

    if (n == m_nodes.end()) {
      snprintf(buf, 100, "Node %u does not exist.", idx);
      throw SRE((string) buf);
    }

    return n->second.get();
}

Edge* Network::get_edge(uint32_t fr, uint32_t to) const
{
    char buf[100];
    auto e = m_edges.find(make_pair(fr, to));

    if (e == m_edges.end()) {
      snprintf(buf, 100, "Edge %u -> %u does not exist.", fr, to);
      throw SRE((string) buf);
    }

    return e->second.get();
}

void Network::remove_node(uint32_t idx, bool force)
{
    char buf[100];
    Node* n = get_node(idx);

    if (!force && n->input_id >= 0) {
      snprintf(buf, 100, "Input node %u cannot be removed.", n->id);
      throw SRE(buf);
    }
    if (!force && n->output_id >= 0) {
      snprintf(buf, 100, "Output node %u cannot be removed.", n->id);
      throw SRE(buf);
    }

    // Any node addition or deletion invalidates the sorted_node_vector.
    sorted_node_vector.clear();
   
    // Remove all synapses to/from this node
    // Note: we just need to remove references to these edges & then remove from the hash table
    // This node's incoming/outgoing vector can be deallocated all together rather than per-edge
    for(auto e : n->incoming)
    {
        Node *from_node = e->from;
        auto f_edge = std::find(from_node->outgoing.begin(), from_node->outgoing.end(), e);
        std::iter_swap(f_edge, from_node->outgoing.end() - 1);
        from_node->outgoing.pop_back();
        m_edges.erase(make_pair(from_node->id, idx));
    }

    for(auto e : n->outgoing)
    {
        Node *to_node = e->to;
        auto t_edge = std::find(to_node->incoming.begin(), to_node->incoming.end(), e);
        std::iter_swap(t_edge, to_node->incoming.end() - 1);
        to_node->incoming.pop_back();
        m_edges.erase(make_pair(idx, to_node->id));
    }

    if(n->input_id >= 0)
        m_inputs[n->input_id] = -1;

    if(n->output_id >= 0)
        m_outputs[n->output_id] = -1;

    // hash table owns the pointer, so this also deconstructs the node
    m_nodes.erase(idx);
}

void Network::remove_edge(uint32_t fr, uint32_t to)
{
    Edge* e = get_edge(fr, to);
    Node* from_node = get_node(fr);
    Node* to_node = get_node(to);

    // get location in node-level vectors
    auto f_edge = std::find(from_node->outgoing.begin(), from_node->outgoing.end(), e);
    auto t_edge = std::find(to_node->incoming.begin(), to_node->incoming.end(), e);

    // TODO: check find to see if it was found or not

    // swap to end & pop
    std::iter_swap(f_edge, from_node->outgoing.end() - 1);
    from_node->outgoing.pop_back();

    std::iter_swap(t_edge, to_node->incoming.end() - 1);
    to_node->incoming.pop_back();

    // removal from hash table must be the last operation
    m_edges.erase(make_pair(fr, to));
}

int Network::add_input(uint32_t idx)
{
    Node *n = get_node(idx);
    char buf[100];

    if (n->input_id >= 0) {
      snprintf(buf, 100, "Node %u was already set as an input (%u).", idx, n->input_id);
      throw SRE(buf);
    }

    n->input_id = m_inputs.size();
    m_inputs.push_back(idx);
    return n->input_id;
}

int Network::add_output(uint32_t idx)
{
    Node *n = get_node(idx);
    char buf[100];
    if (n->output_id >= 0) {
      snprintf(buf, 100, "Node %u was already set as an output (%u).", idx, n->output_id);
      throw SRE(buf);
    }
    n->output_id = m_outputs.size();
    m_outputs.push_back(idx);
    return n->output_id;
}

Node* Network::get_input(int input_id) const
{
    char buf[48];
    int node_id = m_inputs.at(input_id);
    if (node_id < 0) {
      snprintf(buf, 48, "Input %d does not have a node", input_id);
      throw SRE(buf);
    }
    return get_node(node_id);
}

Node* Network::get_output(int output_id) const
{
    char buf[48];
    int node_id = m_outputs.at(output_id);
    if (node_id < 0) {
      snprintf(buf, 48, "Output %d does not have a node", output_id);
      throw SRE(buf);
    }
    return get_node(node_id);
}

int Network::num_inputs() const
{
    return m_inputs.size();
}

int Network::num_outputs() const
{
    return m_outputs.size();
}

NodeMap::iterator Network::begin()
{
    return m_nodes.begin();
}

NodeMap::iterator Network::end()
{
    return m_nodes.end();
}

EdgeMap::iterator Network::edges_begin()
{
    return m_edges.begin();
}

EdgeMap::iterator Network::edges_end()
{
    return m_edges.end();
}

void Network::make_sorted_node_vector()
{
  NodeMap::iterator nit;

  if (sorted_node_vector.size() != 0) return;
  for (nit = begin(); nit != end(); nit++) sorted_node_vector.push_back(nit->second.get());
  sort(sorted_node_vector.begin(), sorted_node_vector.end(), node_comp);
}

size_t Network::num_nodes() const
{
    return m_nodes.size();
}

size_t Network::num_edges() const
{
    return m_edges.size();
}

void Network::set_data(const string& name, const json& data)
{
    m_associated_data[name] = data;
}

json Network::get_data(const string& name) const
{
    if (!m_associated_data.contains(name)) {
      throw SRE((string) "Associated data key '" + name + "' not found");
    }

    return m_associated_data[name];
}

vector<string> Network::data_keys() const
{
    vector<string> keys;

    for(auto& it : m_associated_data.items())
    {
        keys.push_back(it.key());
    }

    return keys;
}

Node* Network::get_random_node(MOA& moa) const
{
    // TODO: Make this good
    auto it = m_nodes.begin();
    std::advance(it, moa.Random_Integer() % num_nodes());
    return it->second.get();
}

Edge* Network::get_random_edge(MOA& moa) const
{
    // TODO: Make this good
    auto it = m_edges.begin();
    std::advance(it, moa.Random_Integer() % num_edges());
    return it->second.get();
}

Node* Network::get_random_input(MOA& moa) const
{
    return get_input(moa.Random_Integer() % num_inputs());
}

Node* Network::get_random_output(MOA& moa) const
{
    return get_output(moa.Random_Integer() % num_outputs());
}

double Network::random_value(MOA& moa, const Property& p)
{
    double rv;

    switch(p.type)
    {
        case Property::Type::DOUBLE:
            rv = (moa.Random_Double() * (p.max_value - p.min_value) + p.min_value);
            break;
        case Property::Type::INTEGER:
            rv = (moa.Random_Integer() % ((int) p.max_value - (int) p.min_value + 1) + p.min_value);
            break;
        case Property::Type::BOOLEAN:
            rv =  (moa.Random_Integer() % 2);
            break;
        default:
            throw SRE("Network::random_value() - Bad case");
            break;
    }

    return rv;
}

void Network::randomize(const json& params)
{
    (void) params;
    // TODO
    //   Allow this function to be a front-end for a family of functions
    throw std::logic_error("Random network generation is not yet implemented.");
}

void Network::randomize_p(const json& params)
{
    (void) params;
    // TODO
    //   Add n hidden nodes
    //   Connect nodes with probability p
    throw std::logic_error("Random network generation is not yet implemented.");
}

void Network::randomize_h(const json& params)
{
    (void) params;
    // TODO
    //   Add n hidden nodes
    //   Randomly connect input -> hidden,
    //                    hidden -> hidden,
    //                    hidden -> output
    throw std::logic_error("Random network generation is not yet implemented.");
}

void Network::prune()
{
    // search state information
    tsl::robin_set<uint32_t> visited;
    std::vector<uint32_t> to_remove;

    // DFS lambdas
    std::function<void(Node*)> traverse_inputs;
    std::function<void(Node*)> traverse_outputs;

    // Any node addition or deletion invalidates the sorted_node_vector.
    // We'll lump prune in here.
    sorted_node_vector.clear();
   
    traverse_inputs = [&traverse_inputs, &visited](Node *n) {
        // check if node has been visited
        auto it = visited.find(n->id);
        if(it == visited.end())
        {
            // mark as visited
            visited.insert(n->id);

            // traverse all incoming edges
            for(Edge *e : n->incoming)
                traverse_inputs(e->from);
        }
    };

    traverse_outputs = [&traverse_outputs, &visited](Node *n) {
        // check if node has been visited
        auto it = visited.find(n->id);
        if(it == visited.end())
        {
            // mark as visited
            visited.insert(n->id);

            // traverse all outgoing edges
            for(Edge *e : n->outgoing)
                traverse_outputs(e->to);
        }
    };

    // Traverse from each input
    for(auto c : m_inputs) traverse_outputs(m_nodes.at(c).get());

    // Check which nodes were visited
    for(auto& elm : m_nodes)
        if(visited.find(elm.first) == visited.end() && (elm.second->is_hidden()))
            to_remove.emplace_back(elm.first);
    
    // Remove nodes which weren't visited
    for(auto c : to_remove)
        remove_node(c, false);

    // Clear state
    visited.clear();
    to_remove.clear();

    // Traverse from each output
    for(auto c : m_outputs) traverse_inputs(m_nodes.at(c).get());

    // Check which nodes were visited
    for(auto& elm : m_nodes)
        if(visited.find(elm.first) == visited.end() && (elm.second->is_hidden()))
            to_remove.emplace_back(elm.first);

    // Remove nodes which weren't visited
    for(auto c : to_remove)
        remove_node(c, false);
}

void Network::randomize_property(MOA& moa, const Property& p, vector<double>& pv)
{
    for(auto i = p.index; i < p.index + p.size; i++)
        pv[i] = random_value(moa, p);
}

void Network::randomize_property(MOA& moa, Node *n, const string& pname)
{
    auto pit = m_properties.nodes.find(pname);
    if (pit == std::end(m_properties.nodes)) {
      throw SRE((string) "Cannot randomize node property '" + pname + "'"); 
    }
    randomize_property(moa, pit->second, n->values);
}

void Network::randomize_property(MOA& moa, Edge *e, const string& pname)
{
    auto pit = m_properties.edges.find(pname);
    if(pit == std::end(m_properties.edges))  {
      throw SRE((string) "Cannot randomize edge property '" + pname + "'"); 
    }
    randomize_property(moa, pit->second, e->values);
}

void Network::randomize_property(MOA& moa, const string& pname)
{
    auto pit = m_properties.networks.find(pname);
    if(pit == std::end(m_properties.networks)) {
      throw SRE((string) "Cannot randomize network property '" + pname + "'"); 
    }
    randomize_property(moa, pit->second, values);
}

void Network::randomize_properties(MOA &moa)
{
    for(auto &p : m_properties.networks)
        randomize_property(moa, p.second, values);
}

void Network::randomize_properties(MOA &moa, Node *n)
{
    for(auto &p : m_properties.nodes)
        randomize_property(moa, p.second, n->values);
}

void Network::randomize_properties(MOA &moa, Edge *e)
{
    for(auto &p : m_properties.edges)
        randomize_property(moa, p.second, e->values);
}

bool Network::is_node_property(const string& name) const
{
    return(m_properties.nodes.find(name) != m_properties.nodes.end());
}

bool Network::is_edge_property(const string& name) const
{
    return(m_properties.edges.find(name) != m_properties.edges.end());
}

bool Network::is_network_property(const string& name) const
{
    return(m_properties.networks.find(name) != m_properties.networks.end());
}

const Property* Network::get_node_property(const string& name) const
{
    auto mit = m_properties.nodes.find(name);
    if (mit == m_properties.nodes.end()) {
      throw SRE((string) "Cannot find node property: " + name);
    }
    return &(mit->second);
}

const Property* Network::get_edge_property(const string& name) const
{
    auto mit = m_properties.edges.find(name);
    if (mit == m_properties.edges.end()) {
      throw SRE((string) "Cannot find edge property: " + name);
    }
    return &(mit->second);
}

const Property* Network::get_network_property(const string& name) const
{
    auto mit = m_properties.networks.find(name);
    if(mit == m_properties.networks.end()) {
      throw SRE((string) "Cannot find network property: " + name);
    }
    return &(mit->second);
}

PropertyPack Network::get_properties() const
{
    return m_properties;
}

void Network::set_properties(const PropertyPack& pp)
{
    PropertyMap::const_iterator pit;
    int i;

    if(!m_nodes.empty() || !m_edges.empty()) 
        throw std::runtime_error("Cannot add properties after the network has elements.");

    m_properties = pp;

    /* Create the values vector and set each element to its max in the property pack */

    values.resize(m_properties.net_vec_size);
    for (pit = m_properties.networks.begin(); pit != m_properties.networks.end(); pit++) {
      for (i = 0; i < pit->second.size; i++) {
        values[pit->second.index+i] = pit->second.max_value;
      }
    }
}

string Network::pretty_nodes() const
{
  size_t i;
  string s;
  bool use_vector;
  NodeMap::const_iterator nit;

  if (m_nodes.size() == 0) return "[]";
  
  use_vector = (sorted_node_vector.size() != 0);

  nit = m_nodes.begin();
  s = "";
  for (i = 0; i < m_nodes.size(); i++) {
    s += ((i == 0) ? "[ " : "  ");
    if (use_vector) {
      s += node_json(sorted_node_vector[i]).dump();
    } else {
      s += node_json(nit->second.get()).dump();
    }
    if (i+1 == m_nodes.size()) {
      s += " ]";
    } else {
      s += ",\n";
    }
    nit++;
  }
  return s;
}

string Network::pretty_edges() const
{
  json j1;
  size_t i, j;
  string s;
  Node *n;
  map <uint32_t, Edge *> m;
  map <uint32_t, Edge *>::iterator mit;
  
  if (num_edges() == 0) return "[]";

  if (sorted_node_vector.size() == 0) {
    j1 = as_json();
    s = "";
    for (i = 0; i < j1["Edges"].size(); i++) {
      s += ((i == 0) ? "[ " : "  ");
      s += j1["Edges"][i].dump();
      if (i+1 == j1["Edges"].size()) {
        s += " ]";
      } else {
        s += ",\n";
      }
    }
    return s;
  } else {
    s = "";
    for (i = 0; i < sorted_node_vector.size(); i++) {
      n = sorted_node_vector[i];
      m.clear();
      for (j = 0; j < n->outgoing.size(); j++) {
        m[n->outgoing[j]->to->id] = n->outgoing[j];
      }
      for (mit = m.begin(); mit != m.end(); mit++) {
        s += ((s.size() == 0) ? "[ " : ",\n  ");
        s += mit->second->as_json().dump();
      }
    }
    s += " ]";
    return s;
  }
}

static void append_and_indent(const string &from, string &to, size_t chars)
{
  size_t i, j;
  char lc;

  lc = '\n';
  for (i = 0; i < from.size(); i++) {
    if (lc == '\n') {
      for (j = 0; j < chars; j++) to.push_back(' ');
    }
    to.push_back(from[i]);
    lc = from[i];
  }
}

static string pretty_json_generic(const json &j)
{
  string t;
  string rv;
  string k;
  string v;
  nlohmann::json::const_iterator pit;
  size_t i;

  if (!j.is_object() && !j.is_array()) {
    return j.dump();
  } else {
    t = j.dump();
    if (t.size() < 80) return t;
    if (j.is_object()) {
      for (pit = j.begin(); pit != j.end(); pit++) {
        rv += (pit == j.begin()) ? " { " : ",\n   ";
        k = "\"" + pit.key() + "\": ";
        v = pretty_json_generic(pit.value());
        if (k.size() + v.size() <= 80) {
          rv += (k + v);
        } else {
          rv += (k + "\n");
          append_and_indent(v, rv, 3);
        }
      }
      rv += "}";
      return rv;
    } else {
      for (i = 0; i < j.size(); i++) {
        rv += (i == 0) ? " [ " : ",\n   ";
        v = pretty_json_generic(j[i]);
        if (v.size() <= 80) {
          rv += v;
        } else {
          append_and_indent(v, rv, 3);
        }
      }
      rv += "]";
      return rv;
    }
  }
}

string Network::pretty_json() const
{
  json j1;
  string s;

  j1 = as_json();
  s = "{ \"Properties\":\n";
  append_and_indent(m_properties.pretty_json(), s, 2);
  s += ",\n \"Nodes\":\n";
  append_and_indent(pretty_nodes(), s, 2);
  s += ",\n \"Edges\":\n";
  append_and_indent(pretty_edges(), s, 2);
  s += ",\n \"Inputs\": ";
  s += j1["Inputs"].dump();
  s += ",\n \"Outputs\": ";
  s += j1["Outputs"].dump();
  s += ",\n \"Network_Values\": ";
  s += j1["Network_Values"].dump();
  s += ",\n \"Associated_Data\":\n";
  append_and_indent(pretty_json_generic(j1["Associated_Data"]), s, 2);
  s += "}";
  return s;
}

}  // End of neuro namespace

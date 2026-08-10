#include "izhikevich.hpp"
#include "utils/json_helpers.hpp"

using namespace std;
using namespace izhikevich;

using neuro::Node;
using neuro::Edge;
using neuro::Spike;
using neuro::Property;
using neuro::PropertyPack;
using neuro::Parameter_Check_Json_T;
using nlohmann::json;

typedef runtime_error SRE;

json Processor::spec = {

  { "min_excitatory_weight", "I" },
  { "max_excitatory_weight", "I" },
  { "min_inhibitory_weight", "I" },
  { "max_inhibitory_weight", "I" },

  { "min_excitatory_delay", "I" },
  { "max_excitatory_delay", "I" },
  { "min_inhibitory_delay", "I" },
  { "max_inhibitory_delay", "I" },

  { "input_scaling_value", "I" }
};

Neuron::Neuron(Node* node, int id)
{
  if (!node) {
    throw (string) "bad node: null pointer";
  }

  /* Regular spiking or RS (excitatory) neuron. */

  if (node->get("exc")) {
    v = -65;
    u = -13;
    a = 0.02;
    b = 0.2;
    c = -65;
    d = 8;

  /* Fast spiking or FS (inhibitory) neuron. */

  } else {
    v = -65;
    u = -13;
    a = 0.1;
    b = 0.2;
    c = -65;
    d = 2;
  }

  this->id = id;
  this->node = node;

  input = 0;

  fire_count = 0;
  tracking = false;
  last_fire_time = -1;
}

void Neuron::update()
{
  v += 0.04 * v * v + 5 * v + 140 - u + input;
  u += a * (b * v - u);
}

void Neuron::fire(Network* net, int time)
{
  size_t i;
  Synapse* s;
  pair<int, double> p;

  /* Send a spike along each of the outgoing
     synapses. */

  for (i = 0; i < synapses.size(); i++) {
    s = synapses[i];
    if (s->delay >= (int) net->events.size()) {
      net->events.resize(s->delay + 1);
    }
    p = make_pair(s->to->id, s->weight);
    net->events[s->delay].push_back(p);
  }

  /* "Reset" neuron. */

  v = c;
  u += d;

  fire_count++;
  last_fire_time = time;
  if (tracking) fire_times.push_back(time);
}

Neuron::~Neuron()
{
  size_t i;

  for (i = 0; i < synapses.size(); i++) {
    delete synapses[i];
  }
}

void Neuron::reset()
{
  v = -65;
  u = -13;

  input = 0;

  fire_count = 0;
  last_fire_time = -1;
  fire_times.clear();
}

Synapse::Synapse(Neuron* to, Edge* edge, bool exc)
{
  if (!edge) {
    throw (string) "bad edge: null pointer";
  }

  /* Which params we use depends on whether the
     synapse is excitatory or inhibitory, which
     depends on whether the presynaptic neuron is
     excitatory or inhibitory. Excitatory neurons
     have only excitatory (outgoing) synapses, and
     similarly for inhibitory neurons. */

  if (exc) {
    weight = edge->get("excitatory_weight");
    delay = edge->get("excitatory_delay");
  } else {
    weight = edge->get("inhibitory_weight");
    delay = edge->get("inhibitory_delay");
  }

  this->to = to;
}

Network::Network(neuro::Network* net)
{
  bool exc;
  size_t i, j;
  Node* n;
  Edge* e;
  Neuron* to;

  net->make_sorted_node_vector();
  neurons.resize(net->num_nodes());

  /* Creating the neurons. */

  for (i = 0; i < net->sorted_node_vector.size(); i++) {
    n = net->sorted_node_vector[i];
    ids[n->id] = i;
    neurons[i] = new Neuron(n, i);
    if (n->is_input()) inputs.push_back(i);
    if (n->is_output()) outputs.push_back(i);
  }

  /* Creating the synapses. */

  for (i = 0; i < net->sorted_node_vector.size(); i++) {
    n = net->sorted_node_vector[i];
    for (j = 0; j < n->outgoing.size(); j++) {
      e = n->outgoing[j];
      to = neurons[ids[e->to->id]];
      exc = neurons[i]->node->get("exc");
      neurons[i]->synapses.push_back(new Synapse(to, e, exc));
    }
  }

  timestep = 0;
  total_fire_count = 0;
  total_accumulate_count = 0;
}

Network::~Network()
{
  size_t i;

  for (i = 0; i < neurons.size(); i++) {
    delete neurons[i];
  }
}

void Network::run(int duration)
{
  int i;
  size_t j;

  /* Reset all the tracking info. */

  for (i = 0; i < (int) neurons.size(); i++) {
    neurons[i]->fire_count = 0;
    neurons[i]->last_fire_time = -1;
    neurons[i]->fire_times.clear();
  }

  /* Simulate for "duration" time steps. */

  for (i = 0; i < duration; i++) {

    /* Process neuron fires. */

    for (j = 0; j < neurons.size(); j++) {
      if (neurons[j]->v >= 30) {
        neurons[j]->fire(this, i);
        total_accumulate_count += neurons[j]->synapses.size();
        total_fire_count++;
      }
    }

    /* Process input spikes and spikes from synapses. */

    if (!events.empty()) {
      for (j = 0; j < events[0].size(); j++) {
        neurons[events[0][j].first]->input += events[0][j].second;
      }
      events.pop_front();
    }

    /* Update the neurons. */

    for (j = 0; j < neurons.size(); j++) {
      neurons[j]->update();
      neurons[j]->input = 0;
    }
  }

  timestep += duration;
}

Neuron* Network::get_neuron(uint32_t node_id)
{
  char buf[20];
  unordered_map<uint32_t, int>::const_iterator it;

  it = ids.find(node_id);

  if (it == ids.end()) {
    snprintf(buf, 20, "%d", node_id);
    throw (string) "bad node id: " + buf;
  }

  return neurons[it->second];
}

Neuron* Network::get_output(int output_id)
{
  char buf[20];

  if (output_id < 0 || output_id >= (int) outputs.size()) {
    snprintf(buf, 20, "%d", output_id);
    throw (string) "bad output id: " + buf;
  }

  return neurons[outputs[output_id]];
}

void Network::reset()
{
  size_t i;

  for (i = 0; i < neurons.size(); i++) {
    neurons[i]->reset();
  }

  events.clear();
  total_fire_count = 0;
  total_accumulate_count = 0;
  timestep = 0;
}

Processor::Processor(json& arg)
{
  string s;
  json::const_iterator jit;

  try {
    Parameter_Check_Json_T(arg, spec);
  } catch (const SRE& e) {
    s = (string) "izhikevich::Processor::Processor(): " + e.what();
    throw SRE(s);
  }

  /* These default values may not be appropriate for
     all applications, encoding strategies, etc. They
     reflect to some degree what Izhikevich has in his
     MATLAB codes (which reflects a particular use case,
     i.e., application). You can use the bayes utility
     in our framework, for example, to try to optimize
     these (along with the encoding strategy, etc., for
     your particular application. */

  params = json::object();

  params["min_excitatory_weight"] = 1;
  params["max_excitatory_weight"] = 40;
  params["min_inhibitory_weight"] = -20;
  params["max_inhibitory_weight"] = -1;

  params["min_excitatory_delay"] = 1;
  params["max_excitatory_delay"] = 20;
  params["min_inhibitory_delay"] = 1;
  params["max_inhibitory_delay"] = 1;

  params["input_scaling_value"] = 40;

  /* Overwrite the default values with any values
     specified in the param file. */

  for (jit = arg.begin(); jit != arg.end(); jit++) {
    params[jit.key()] = arg[jit.key()];
  }

  /* Sanity check the param values. (I'm using else if's
     rather than if's because it looks nicer.) */

  try {

    if (!params["min_excitatory_weight"].is_number_integer() ||
         params["min_excitatory_weight"] < 1) {
      throw (string) "bad parameter: \"min_excitatory_weight\": must "
                     "be a positive integer";
    } else if (!params["max_excitatory_weight"].is_number_integer() ||
                params["max_excitatory_weight"] < 1) {
      throw (string) "bad parameter: \"max_excitatory_weight\": must "
                     "be a positive integer";

    } else if (!params["min_inhibitory_weight"].is_number_integer() ||
                params["min_inhibitory_weight"] > -1) {
      throw (string) "bad parameter: \"min_inhibitory_weight\": must "
                     "be a negative integer";
    } else if (!params["max_inhibitory_weight"].is_number_integer() ||
                params["max_inhibitory_weight"] > -1) {
      throw (string) "bad parameter: \"max_inhibitory_weight\": must "
                     "be a negative integer";

    } else if (!params["min_excitatory_delay"].is_number_integer() ||
                params["min_excitatory_delay"] < 1) {
      throw (string) "bad parameter: \"min_excitatory_delay\": must "
                     "be a positive integer";
    } else if (!params["max_excitatory_delay"].is_number_integer() ||
                params["max_excitatory_delay"] < 1) {
      throw (string) "bad parameter: \"max_excitatory_delay\": must "
                     "be a positive integer";

    } else if (!params["min_inhibitory_delay"].is_number_integer() ||
                params["min_inhibitory_delay"] < 1) {
      throw (string) "bad parameter: \"min_inhibitory_delay\": must "
                     "be a positive integer";
    } else if (!params["max_inhibitory_delay"].is_number_integer() ||
                params["max_inhibitory_delay"] < 1) {
      throw (string) "bad parameter: \"max_inhibitory_delay\": must "
                     "be a positive integer";

    } else if (params["min_excitatory_weight"] >
               params["max_excitatory_weight"]) {
      throw (string) "bad parameter: \"min_excitatory_weight\": must "
                     "be less than or equal to "
                     "\"max_excitatory_weight\"";
    } else if (params["min_inhibitory_weight"] >
               params["max_inhibitory_weight"]) {
      throw (string) "bad parameter: \"min_inhibitory_weight\": must "
                     "be less than or equal to "
                     "\"max_inhibitory_weight\"";

    } else if (params["min_excitatory_delay"] >
               params["max_excitatory_delay"]) {
      throw (string) "bad parameter: \"min_excitatory_delay\": must "
                     "be less than or equal to "
                     "\"max_excitatory_delay\"";
    } else if (params["min_inhibitory_delay"] >
               params["max_inhibitory_delay"]) {
      throw (string) "bad parameter: \"min_inhibitory_delay\": must "
                     "be less than or equal to "
                     "\"max_inhibitory_delay\"";
    }
  } catch (const string &s2) {
    throw SRE("izhikevich::Processor::Processor(): " + s2);
  }

  input_scaling_value = params["input_scaling_value"];

  properties = json::object();
  properties["threshold_inclusive"] = true;
  properties["input_scaling_value"] = input_scaling_value;
  properties["binary_input"] = false;
  properties["spike_raster_info"] = true;
  properties["plasticity"] = "none";
  properties["run_time_inclusive"] = false;

  /* We have an integration delay because a
     neuron that fires at time t actually has
     its charge reach 30 or more at time t - 1,
     but we don't learn about it until time t. */

  properties["integration_delay"] = true;

  /* "exc" is whether a neuron is excitatory
     or inhibitory. */

  ppack.add_node_property("exc", 0, 1, Property::Type::BOOLEAN);

  /* Having weights be integers at least for now. */

  ppack.add_edge_property("excitatory_weight",
                          params["min_excitatory_weight"],
                          params["max_excitatory_weight"],
                          Property::Type::INTEGER);
  ppack.add_edge_property("inhibitory_weight",
                          params["min_inhibitory_weight"],
                          params["max_inhibitory_weight"],
                          Property::Type::INTEGER);

  ppack.add_edge_property("excitatory_delay",
                          params["min_excitatory_delay"],
                          params["max_excitatory_delay"],
                          Property::Type::INTEGER);
  ppack.add_edge_property("inhibitory_delay",
                          params["min_inhibitory_delay"],
                          params["max_inhibitory_delay"],
                          Property::Type::INTEGER);
}

Processor::~Processor()
{
  unordered_map<int, Network *>::iterator it;

  for (it = networks.begin(); it != networks.end(); it++) {
    delete it->second;
  }
}

bool Processor::load_network(neuro::Network* network, int network_id)
{
  string s;

  try {
    if (!network) {
      throw (string) "bad network: null pointer";
    }

    if (network->get_properties().as_json() != ppack.as_json()) {
      throw (string) "bad network: network and processor properties "
                     "do not match";
    }

    clear(network_id);
    networks[network_id] = new Network(network);
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::load_network(): " + s);
  }

  return true;
}

bool Processor::load_networks(vector<neuro::Network *>& networks)
{
  size_t i;

  for (i = 0; i < networks.size(); i++) {
    load_network(networks[i], i);
  }

  return true;
}

void Processor::clear(int network_id)
{
  unordered_map<int, Network *>::iterator it;

  it = networks.find(network_id);

  if (it != networks.end()) {
    networks.erase(it);
  }
}

void Processor::apply_spike(
  const Spike& spike,
  bool normalized,
  int network_id)
{
  char buf[20];
  double val;
  string s;
  Network* net;
  pair<int, double> p;

  try {

    net = get_network(network_id);

    /* Am I right in the thought that spike.id
       is an input id and not the node id? */

    if (spike.id < 0 || spike.id >= (int) net->inputs.size()) {
      snprintf(buf, 20, "%d", spike.id);
      throw (string) "bad input id: " + buf;
    }

    if (spike.time < 0) {
      snprintf(buf, 20, "%lf", spike.time);
      throw (string) "bad spike time: " + buf;
    }

    if (normalized) {
      if (spike.value < 0 || spike.value > 1) {
        snprintf(buf, 20, "%lf", spike.value);
        throw (string) "bad spike value: " + buf + ": "
                       "value must be in [0, 1]";
      }
      val = spike.value * input_scaling_value;
    } else {
      val = spike.value;
    }
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::apply_spike(): " + s);
  }

  if (spike.time >= (int) net->events.size()) {
    net->events.resize(spike.time + 1);
  }
  p = make_pair(net->inputs[spike.id], val);
  net->events[spike.time].push_back(p);
}

void Processor::apply_spike(
  const Spike& spike,
  const vector<int>& network_ids,
  bool normalized)
{
  size_t i;

  for (i = 0; i < network_ids.size(); i++) {
    apply_spike(spike, normalized, network_ids[i]);
  }
}

void Processor::apply_spikes(
  const vector<Spike>& spikes,
  bool normalized,
  int network_id)
{
  size_t i;

  for (i = 0; i < spikes.size(); i++) {
    apply_spike(spikes[i], normalized, network_id);
  }
}

void Processor::apply_spikes(
  const vector<Spike>& spikes,
  const vector<int>& network_ids,
  bool normalized)
{
  size_t i;

  for (i = 0; i < spikes.size(); i++) {
    apply_spike(spikes[i], network_ids, normalized);
  }
}

void Processor::run(double duration, int network_id)
{
  Network* net;

  /* Run calls with a duration less than or equal to
     zero do not affect the network state. */

  if (duration <= 0) return;

  try {
    net = get_network(network_id);
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::run(): " + s);
  }

  net->run(duration);
}

void Processor::run(double duration, const vector<int>& network_ids)
{
  size_t i;

  for (i = 0; i < network_ids.size(); i++) {
    run(duration, network_ids[i]);
  }
}

double Processor::get_time(int network_id)
{
  try {
    return get_network(network_id)->timestep;
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::get_time(): " + s);
  }
}

bool Processor::track_output_events(
  int output_id,
  bool track,
  int network_id)
{
  Neuron *n;

  try {
    n = get_network(network_id)->get_output(output_id);
    n->tracking = track;
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::track_output_events(): " + s);
  }

  return true;
}

bool Processor::track_neuron_events(
  uint32_t node_id,
  bool track,
  int network_id)
{
  Neuron* n;

  try {
    n = get_network(network_id)->get_neuron(node_id);
    n->tracking = track;
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::track_neuron_events(): " + s);
  }

  return true;
}

double Processor::output_last_fire(int output_id, int network_id)
{
  Neuron* n;

  try {
    n = get_network(network_id)->get_output(output_id);
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::output_last_fire(): " + s);
  }

  return n->last_fire_time;
}

vector<double> Processor::output_last_fires(int network_id)
{
  size_t i;
  Network* net;
  vector<double> times;

  try {
    net = get_network(network_id);
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::output_last_fires(): " + s);
  }

  times.resize(net->outputs.size());
  for (i = 0; i < net->outputs.size(); i++) {
    times[i] = net->neurons[net->outputs[i]]->last_fire_time;
  }

  return times;
}

int Processor::output_count(int output_id, int network_id)
{
  Neuron* n;

  try {
    n = get_network(network_id)->get_output(output_id);
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::output_count(): " + s);
  }

  return n->fire_count;
}

vector<int> Processor::output_counts(int network_id)
{
  size_t i;
  Network* net;
  vector<int> counts;

  try {
    net = get_network(network_id);
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::output_counts(): " + s);
  }

  counts.resize(net->outputs.size());

  for (i = 0; i < net->outputs.size(); i++) {
    counts[i] = net->neurons[net->outputs[i]]->fire_count;
  }

  return counts;
}

vector<double> Processor::output_vector(
  int output_id,
  int network_id)
{
  Neuron* n;

  try {
    n = get_network(network_id)->get_output(output_id);
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::output_vector(): " + s);
  }

  return n->fire_times;
}

vector< vector<double> > Processor::output_vectors(int network_id)
{
  size_t i;
  Network* net;
  vector< vector<double> > times;

  try {
    net = get_network(network_id);
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::output_vectors(): " + s);
  }

  times.resize(net->outputs.size());

  for (i = 0; i < net->outputs.size(); i++) {
    times[i] = net->neurons[net->outputs[i]]->fire_times;
  }

  return times;
}

long long Processor::total_neuron_counts(int network_id)
{
  long long count;
  Network* net;

  try {
    net = get_network(network_id);
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::total_neuron_counts(): " + s);
  }

  count = net->total_fire_count;
  net->total_fire_count = 0;
  return count;
}

long long Processor::total_neuron_accumulates(int network_id)
{
  long long count;
  string s;
  Network* net;

  try {
    net = get_network(network_id);
  } catch (const string& s2) {
    s = "izhikevich::Processor::total_neuron_accumulates(): " + s;
    throw SRE(s);
  }

  count = net->total_accumulate_count;
  net->total_accumulate_count = 0;
  return count;
}

vector<int> Processor::neuron_counts(int network_id)
{
  size_t i;
  Network* net;
  vector<int> counts;

  try {
    net = get_network(network_id);
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::neuron_counts(): " + s);
  }

  counts.resize(net->neurons.size());

  for (i = 0; i < net->neurons.size(); i++) {
    counts[i] = net->neurons[i]->fire_count;
  }

  return counts;
}

vector<double> Processor::neuron_last_fires(int network_id)
{
  size_t i;
  Network* net;
  vector<double> times;

  try {
    net = get_network(network_id);
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::neuron_last_fires(): " + s);
  }

  times.resize(net->neurons.size());

  for (i = 0; i < net->neurons.size(); i++) {
    times[i] = net->neurons[i]->last_fire_time;
  }

  return times;
}

vector< vector<double> > Processor::neuron_vectors(int network_id)
{
  size_t i;
  Network* net;
  vector< vector<double> > times;

  try {
    net = get_network(network_id);
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::neuron_vectors(): " + s);
  }

  times.resize(net->neurons.size());

  for (i = 0; i < net->neurons.size(); i++) {
    times[i] = net->neurons[i]->fire_times;
  }

  return times;
}

vector<double> Processor::neuron_charges(int network_id)
{
  size_t i;
  double v;
  Network* net;
  vector<double> charges;

  try {
    net = get_network(network_id);
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::neuron_charges(): " + s);
  }

  charges.resize(net->neurons.size());

  for (i = 0; i < net->neurons.size(); i++) {
    v = net->neurons[i]->v;
    charges[i] = (v > 30) ? 30 : v;
  }

  return charges;
}

void Processor::synapse_weights(
  vector<uint32_t>& pres,
  vector<uint32_t>& posts,
  vector<double>& vals,
  int network_id)
{
  size_t i, j;
  Network* net;

  try {
    net = get_network(network_id);
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::synapse_weights(): " + s);
  }

  pres.clear();
  posts.clear();
  vals.clear();

  for (i = 0; i < net->neurons.size(); i++) {
    for (j = 0; j < net->neurons[i]->synapses.size(); j++) {
      pres.push_back(net->neurons[i]->node->id);
      posts.push_back(net->neurons[i]->synapses[j]->to->node->id);
      vals.push_back(net->neurons[i]->synapses[j]->weight);
    }
  }
}

void Processor::clear_activity(int network_id)
{
  try {
    get_network(network_id)->reset();
  } catch (const string& s) {
    throw SRE("izhikevich::Processor::clear_activity(): " + s);
  }
}

PropertyPack Processor::get_network_properties() const
{
  return ppack;
}

json Processor::get_processor_properties() const
{
  return properties;
}

json Processor::get_params() const
{
  return params;
}

string Processor::get_name() const
{
  return "izhikevich";
}

Network* Processor::get_network(int network_id)
{
  char buf[20];
  unordered_map<int, Network *>::const_iterator it;

  it = networks.find(network_id);

  if (it == networks.end()) {
    snprintf(buf, 20, "%d", network_id);
    throw (string) "bad network_id: " + buf;     
  }
  return it->second;
}

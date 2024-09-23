#include <iostream>
#include <cmath>
#include "risp.hpp"
#include "utils/json_helpers.hpp"
#include <random>

typedef std::runtime_error SRE;
using namespace std;

namespace risp {

static json risp_spec = {
  { "min_weight", "D" },
  { "max_weight", "D`" },
  { "max_delay", "I" },
  { "min_threshold", "D" },
  { "max_threshold", "D" },
  { "min_potential", "D" },
  { "noisy_stddev", "D" },
  { "spike_value_factor", "D" },
  { "leak_mode", "S" },            /* "all", "none", "configurable" */
  { "fire_like_ravens", "B" },
  { "run_time_inclusive", "B" },
  { "threshold_inclusive", "B" },
  { "fire_like_ravens", "B" },
  { "discrete", "B" },
  { "weights", "A"},
  { "inputs_from_weights", "B"},
  { "noisy_seed", "I" },
  { "stds", "A"},
    { "Necessary", { "max_delay", 
                     "min_threshold",
                     "max_threshold",
                     "min_potential",
                     "discrete" } } };

/** initialization for Neuron, Synapse  */
Neuron::Neuron(uint32_t node_id, double t, bool l) 
  : charge(0),
    threshold(t),
    last_check(-1),
    last_fire(-1),
    fire_counts(0),
    leak(l),
    id(node_id),
    check(false) {};

Synapse::Synapse(double w, uint32_t d, Neuron* to_n) : weight(w), to(to_n), delay(d) {};

void Neuron::perform_fire(int time)
{
  if (track) fire_times.push_back(time);
  last_fire = time;
  fire_counts++;
  charge = 0;
}

Network::Network(neuro::Network *net, 
                 double _spike_value_factor, 
                 double _min_potential, 
                 char leak,
                 bool _run_time_inclusive,
                 bool _threshold_inclusive,
                 bool _fire_like_ravens,
                 bool _discrete, 
                 bool _inputs_from_weights, 
                 uint32_t _noisy_seed,
                 double _noisy_stddev,
                 vector <double> & _weights, 
                 vector < double> & _stds) {

  size_t i;
  neuro::Node *node;
  neuro::Edge *edge;
  EdgeMap::iterator eit;
  Neuron *n;
  leak_mode = leak;
  bool neuron_leak;

  spike_value_factor = _spike_value_factor;
  min_potential = _min_potential;
  run_time_inclusive = _run_time_inclusive;
  threshold_inclusive = _threshold_inclusive;
  fire_like_ravens = _fire_like_ravens;
  noisy_seed = _noisy_seed;
  noisy_stddev = _noisy_stddev;
  weights = _weights;
  stds = _stds;
  discrete = _discrete;
  inputs_from_weights = _inputs_from_weights;
  overall_run_time = 0;
  neuron_fire_counter = 0;
  neuron_accum_counter = 0;
  rng.Seed(noisy_seed, "noisy_risp");

  /* Add neurons */
  net->make_sorted_node_vector();

  for(i = 0; i < net->sorted_node_vector.size(); i++) {
    node = net->sorted_node_vector[i];

    if (leak_mode == 'c') {
      neuron_leak = (node->get("Leak") != 0);
    } else {
      neuron_leak = (leak_mode == 'a');
    }

    n = add_neuron(node->id, node->get("Threshold"), neuron_leak);
    if (node->is_input()) add_input(node->id, node->input_id);
    if (node->is_output()) add_output(node->id, node->output_id);

    sorted_neuron_vector.push_back(n);
  }

  /* Add synpases */
  for (eit = net->edges_begin(); eit != net->edges_end(); ++eit) {
    edge = eit->second.get();
    add_synpase(edge->from->id, edge->to->id, edge->get("Weight"), edge->get("Delay"));
  }
}

Neuron* Network::get_neuron(uint32_t node_id) 
{
  unordered_map <uint32_t, Neuron*>::const_iterator it;
  char buf[200];

  it = neuron_map.find(node_id);

  if (it == neuron_map.end()) {
    snprintf(buf, 200, "risp::Network::get_neuron() - %u is not in the neuron map\n", node_id);
    throw SRE((string) buf);
  }
  return it->second;
}

bool Network::is_neuron(uint32_t node_id) {
  return neuron_map.find(node_id) != neuron_map.end();
}

bool Network::is_valid_input_id(int input_id) {
  return !(input_id < 0 || input_id >= (int) inputs.size() || inputs[input_id] == -1);
}

bool Network::is_valid_output_id(int output_id) {
  return !(output_id < 0 || output_id >= (int) outputs.size() || outputs[output_id] == -1);
}

Neuron* Network::add_neuron(uint32_t node_id, double threshold, bool leak) {
  Neuron *n;
  char buf[200];

  if (is_neuron(node_id)) {
    snprintf(buf, 200, "risp::Neuron::add_neuron() - %u is already in the neuron map\n", node_id);
    throw SRE((string) buf);
  }
  n = new Neuron(node_id, threshold, leak);

  /* JSP: I'm not a big fan of this hack, 
     but I'd rather do this than put an if
     statement before every threshold check.  */

  if (!threshold_inclusive) {
    n->threshold = (discrete) ? (n->threshold+1) : (n->threshold + 0.0000001);
  }

  neuron_map[node_id] = n;
  return n;
}

Network::~Network() {
  size_t i;
  Neuron *n;
  unordered_map <uint32_t, Neuron*>::const_iterator it;

  for (it = neuron_map.begin(); it != neuron_map.end(); ++it) {
    n = it->second;
    for (i = 0; i < n->synapses.size(); i++) delete n->synapses[i];
    delete n;
  }
}

Synapse* Network::add_synpase(uint32_t from_id, uint32_t to_id, double weight, uint32_t delay) {
  Neuron *from, *to;
  Synapse *syn;
  unordered_map <uint32_t, Neuron*>::const_iterator it;
  char buf[200];

  if (!is_neuron(from_id)) {
    snprintf(buf, 200, "risp::Network::add_synpase() - node %u does not exist", from_id);
    throw SRE((string) buf);
  }
  if (!is_neuron(to_id)) {
    snprintf(buf, 200, "risp::Network::add_synpase() - node %u does not exist", to_id);
    throw SRE((string) buf);
  }

  from = get_neuron(from_id);
  to = get_neuron(to_id);
  
  syn = new Synapse(weight, delay, to);
  from->synapses.push_back(syn);

  return syn;
}


void Network::add_input(uint32_t node_id, int input_id) 
{
  char buf[200];
  
  if (!is_neuron(node_id)) {
    snprintf(buf, 200, "risp::Network::add_input() - node %u does not exist", node_id);
    throw SRE((string) buf);
  }
  if (input_id < 0) throw SRE("risp::Network::add_input() - input_id < 0");
  if (input_id >= (int) inputs.size()) inputs.resize(input_id + 1, -1);
  inputs[input_id] = node_id;
}


void Network::add_output(uint32_t node_id, int output_id) 
{
  char buf[200];
  if (!is_neuron(node_id)) {
    snprintf(buf, 200, "risp::Network::add_output() - node %u does not exist", node_id);
    throw SRE((string) buf);
  }
  if (output_id < 0) throw SRE("risp::Network::add_output() - output_id < 0");
  if (output_id >= (int) outputs.size()) outputs.resize(output_id + 1, -1);
  outputs[output_id] = node_id;
}

void Network::process_events(uint32_t time) 
{
  size_t i, j;
  Neuron *n;
  Synapse *syn;
  size_t to_time;
  size_t events_size;
  double weight;

  /*
    the number of events may be very big, we want to deallocate the vector.
    cons:
      vector will perform a lot reallocation.
    pros:
      It will allow you to do more threads in the cluster. Think about this - 
      Let's assume at each timestep, it has roughly 50000 events.
      That wil consume 50000 * timestep * sizeof(pair<Neuron*, double>) bytes.
      When the timestep is very big, which will be for optimized whetstone's conv2d, 
      It may uses several GB memory.

   The move constructor is going to deallocate the memory.
   At the end of function, es should be free
  */
  const vector<std::pair <Neuron*, double>> es = std::move(events[time]);

  /* Cause neurons to fire if we're firing like RAVENS */

  for (i = 0; i < to_fire.size(); i++) to_fire[i]->perform_fire(time);
  neuron_fire_counter += to_fire.size();
  to_fire.clear();
  
  /* apply leak / reset minimum charge before the events happen */

  for (i = 0; i < es.size(); i++) {
    n = es[i].first;
    if (n->leak) n->charge = 0;
    if (n->charge < min_potential) n->charge = min_potential;
  }

  /* collect charges */

  for (i = 0; i < es.size(); i++) {
    n = es[i].first;
    n->check = true;
    n->charge += es[i].second;
    neuron_accum_counter++;
  }

  /* (CZ): I store events vector size onto events_size. 
     I think this is more efficient than using .size() call a lot times.
     (JSP doesn't think it matters.)  */

  events_size = events.size();

  /* determine if neuron fires */
  for (i = 0; i < es.size(); i++) {
    
    n = es[i].first;
    if (n->check == true) {

      /* fire */
      if (n->charge >= n->threshold) {
        for (j = 0; j < n->synapses.size(); j++) {
          syn = n->synapses[j];
          to_time = time + syn->delay;

          if (to_time >= events_size) {
            events_size = to_time + 1;
            events.resize(events_size);
          }

          if (weights.size() == 0) {
            weight = syn->weight;
          } else if (stds.size() == 0) {
            weight = weights[int(syn->weight)];
          } else {
            weight = rng.Random_Normal(weights[int(syn->weight)], stds[int(syn->weight)]);
          }
          if (noisy_stddev != 0) weight = rng.Random_Normal(weight, noisy_stddev);

          events[to_time].push_back(make_pair(syn->to, weight));
          
        }

        if (fire_like_ravens) {
          to_fire.push_back(n);
        } else {
          neuron_fire_counter++;
          n->perform_fire(time);
        }
      }
      n->check = false;
    }
  }
}


void Network::clear_activity() {

  Neuron *n;
  size_t i;
  for (i = 0; i < sorted_neuron_vector.size(); i++) {
    n = sorted_neuron_vector[i];
    n->last_fire = -1;
    n->fire_counts = 0;
    n->fire_times.clear();  // JSP should clear regardless of tracking.
    n->charge = 0;
    n->last_check = -1;
  }

  events.clear();
  to_fire.clear();
  overall_run_time = 0;
}

void Network::apply_spike(const Spike& s) 
{
  Neuron *n;
  double v;
  char buf[24];
  size_t index;

  if (s.value < 0 || s.value > 1) {
    snprintf(buf, 24, "%lg", s.value);
    throw SRE((string) "risp::Network::apply_spike() - value (" + buf + ") must be in [0,1].");
  }

  if (!is_valid_input_id(s.id)) {
    snprintf(buf, 24, "%d", s.id);
    throw SRE((string) "risp::Network::apply_spike() - input_id " + buf + " is not valid");
  }

  n = get_neuron(inputs[s.id]);
  if (s.time >= events.size()) events.resize(s.time + 1);
  if (inputs_from_weights) {
    index = (s.value * weights.size());
    if (index >= weights.size()) index = weights.size()-1;
    if (stds.size() != 0) {
      v = rng.Random_Normal(weights[index], stds[index]);
    } else {
      v = weights[index];
    }
  } else {
    v = (discrete) ? floor(s.value * spike_value_factor) : s.value * spike_value_factor;
  }
  if (noisy_stddev != 0) v = rng.Random_Normal(v, noisy_stddev);
  events[s.time].push_back(std::make_pair(n,v));
}

void Network::clear_tracking_info()
{
  size_t i;
  Neuron *n;

  for (i = 0; i < sorted_neuron_vector.size(); i++) {
    n = sorted_neuron_vector[i];
    n->last_fire = -1;
    n->fire_counts = 0;
    n->fire_times.clear();  // Doesn't matter if tracking is on or off.
  }
}

void Network::run(double duration) {
  uint32_t i;
  Neuron *n;
  int run_time;

  if (duration < 0) throw SRE("risp::Network::run() - duration < 0");
    
  /* if clear_activity get called, we don't want to clear tracking info again. */
  if (overall_run_time != 0) clear_tracking_info();

  run_time = (run_time_inclusive) ? duration : duration-1;
  overall_run_time += (run_time+1);

  /* expand the event buffer */
  if ((int) events.size() <= run_time) {
    events.resize(run_time + 1);
  }

  for (i = 0; i <= (uint32_t) run_time; i++) {
    process_events(i);
  }

  /* shift extra spiking events to the beginning from the previous run() call*/
  if ((int) events.size() > run_time + 1) {
    for (i = run_time + 1; i < events.size(); i++) {
      events[i - run_time - 1] = std::move(events[i]);   // Google tells me this is O(1)

    }
  }

  /* Deal with leak/non-negative charge  at the end of the run, 
     so that if you pull neuron charges, they will be correct */

  for (i = 0; i < sorted_neuron_vector.size(); i++) {
    n = sorted_neuron_vector[i];
    if (n->leak) n->charge = 0;
    if (n->charge < min_potential) n->charge = min_potential;
  }
}

int Network::output_count(int output_id) 
{
  char buf[200];
  if (!is_valid_output_id(output_id)) {
    snprintf(buf, 200, "risp::Network::output_count() - output id %u is not valid", output_id);
    throw SRE((string) buf);
  }
  return neuron_map[outputs[output_id]]->fire_counts;
}

vector <int> Network::output_counts() {
  size_t i;
  vector <int> rv;
  for (i = 0; i < outputs.size(); i++) {
    if (outputs[i] != -1) rv.push_back(neuron_map[outputs[i]]->fire_counts);
  }
  return rv;
}


double Network::get_time() {
  return (double) overall_run_time;
}

long long Network::total_neuron_accumulates() 
{
  long long rv;
  
  rv = neuron_accum_counter;
  neuron_accum_counter = 0;
  return rv;
}

long long Network::total_neuron_counts() 
{
  long long rv;
  
  rv = neuron_fire_counter;
  neuron_fire_counter = 0;
  return rv;
}

bool Network::track_output_events(int output_id, bool track) {
  if (!is_valid_output_id(output_id)) return false;
  neuron_map[outputs[output_id]]->track = track;
  return true;
}

bool Network::track_neuron_events(uint32_t node_id, bool track) {
  if(!is_neuron(node_id)) return false;
  neuron_map[node_id]->track = track;
  return true;
}


double Network::output_last_fire(int output_id) 
{
  char buf[100];

  if (!is_valid_output_id(output_id)) {
    snprintf(buf, 100, "risp::Network::output_last_fire() - output_id %u is not valid", output_id);
    throw SRE((string) buf);
  }
  return neuron_map[outputs[output_id]]->last_fire;
}

vector <double> Network::output_last_fires() {
  size_t i;
  vector <double> rv;
  for (i = 0; i < outputs.size(); i++) {
    if(outputs[i] != -1) rv.push_back(neuron_map[outputs[i]]->last_fire);
  }
  return rv;
}


vector <double> Network::output_vector(int output_id) 
{
  char buf[100];
  if (!is_valid_output_id(output_id)) {
    snprintf(buf, 100, "risp::Network::output_last_vector() - output_id %u is not valid", 
             output_id);
    throw SRE((string) buf);
  }
  return neuron_map[outputs[output_id]]->fire_times;    // If tracking is turned off, this is empty.
}

vector < vector <double> > Network::output_vectors() {
  
  size_t i;
  Neuron *n;
  vector < vector <double> > rv;

  for (i = 0; i < outputs.size(); i++) {
    if (outputs[i] != -1) {
      n = neuron_map[outputs[i]];
      rv.push_back(n->fire_times);     // If tracking is turned off, this will be empty.
    }
  }
  return rv;

}

vector <int> Network::neuron_counts() {
  size_t i;
  vector <int> rv;

  for (i = 0; i < sorted_neuron_vector.size(); i++) {
    rv.push_back(sorted_neuron_vector[i]->fire_counts);
  }

  return rv;
}

vector < vector <double> > Network::neuron_vectors() {
  vector < vector <double> > rv;
  Neuron *n;
  size_t i;
  
  for (i = 0; i < sorted_neuron_vector.size(); i++) {
    n = sorted_neuron_vector[i];
    rv.push_back(n->fire_times);   // JSP: If tracking is turned off, this will be empty.
  }
  return rv;
}

vector < double > Network::neuron_charges() {
  vector < double > rv;
  Neuron *n;
  size_t i;
  
  for (i = 0; i < sorted_neuron_vector.size(); i++) {
    n = sorted_neuron_vector[i];
    rv.push_back(n->charge);
  }
  return rv;
}


vector <double> Network::neuron_last_fires() {
  vector <double> rv;
  size_t i;

  for (i = 0; i < sorted_neuron_vector.size(); i++) {
    rv.push_back(sorted_neuron_vector[i]->last_fire);
  }
  return rv;
}

void Network::synapse_weights(vector <uint32_t> &pres,
                              vector <uint32_t> &posts,
                              vector <double> &vals) {
  size_t i, j;
  Neuron *n;

  pres.clear();
  posts.clear();
  vals.clear();

  for (i = 0; i < sorted_neuron_vector.size(); i++) {
    n = sorted_neuron_vector[i];
    for (j = 0; j < n->synapses.size(); j++) {
      pres.push_back(n->id);
      posts.push_back(n->synapses[j]->to->id);
      vals.push_back(n->synapses[j]->weight);
    }
  }
}

static bool is_integer(double v)
{
  int iv;

  iv = v;
  return (iv == v);
}

Processor::Processor(json &params) 
{
  string estring;
  size_t i;

  if (params.contains("input_scaling_value")) {
    estring =  "RISP: input_scaling_value is no longer supported.\n";
    estring += "      Please change that parameter to spike_value_factor.\n";
    throw SRE(estring);
  }

  if (params.contains("non_negative_charge")) {
    estring =  "RISP: non_negative_charge is no longer supported.\n";
    estring += "      Instead, min_potential is a required parameter.\n";
    estring += "      Please set min_potential to zero.\n";
    estring += "      Please also see utils/risp_08_2024.\n";
    throw SRE(estring);
  }

  if (params.contains("specific_weights")) {
    estring =  "RISP: specific_weights is no longer supported.\n";
    estring += "      Instead, you simply set the weights array, and it will be used.\n";
    estring += "      Please also see utils/risp_08_2024.\n";
    throw SRE(estring);
  }

  if (params.contains("noisy_weights")) {
    estring =  "RISP: noisy_weights is no longer supported.\n";
    estring += "      Instead, you simply set the stds array, and it will be used.\n";
    estring += "      Please also see utils/risp_08_2024.\n";
    throw SRE(estring);
  }

  Parameter_Check_Json_T(params, risp_spec);

  /* Default params */

  min_delay = 1;
  leak_mode = "none";
  run_time_inclusive = false;
  threshold_inclusive = true;
  fire_like_ravens = false;
  noisy_seed = 0;
  noisy_stddev = 0;
  inputs_from_weights = false;

  /* You don't have to check for these, because they are required in the JSON. */

  max_delay = params["max_delay"];
  min_threshold = params["min_threshold"];
  max_threshold = params["max_threshold"];
  min_potential = params["min_potential"];
  discrete = params["discrete"];

  /* Handle weights + min_weights + max_weights + inputs_from_weight. */

  if (params.contains("weights") && params["weights"].size() > 0) {

    weights = params["weights"].get< vector <double> >(); 

    if (params.contains("min_weight")) throw SRE("RISP: Cannot have weights and min_weight.");
    if (params.contains("max_weight")) throw SRE("RISP: Cannot have weights and max_weight.");
    if (!params.contains("inputs_from_weights")) {
      throw SRE("RISP: If you specify weights, then you must specify inputs_from_weights.");
    }

    for (i = 1; i < weights.size(); i++) {
      if (weights[i] < weights[i-1]) {
        throw SRE("RISP: If you specify weights, then they must be sorted.\n");
      }
      if (discrete && weights[i] - ((int) weights[i]) != 0) {
        throw SRE("RISP: Since discrete is true, weights must be integers.\n");
      }
    }

    inputs_from_weights = params["inputs_from_weights"];

    if (!inputs_from_weights && !params.contains("spike_value_factor")) {
      throw SRE("RISP: If inputs_from_weights is false, you must specify spike_value_factor.");
    }

    if (inputs_from_weights && params.contains("spike_value_factor")) {
      throw SRE("RISP: If inputs_from_weights is true, you cannot specify spike_value_factor.");
    }
 
    min_weight = -9999;
    max_weight = -9999;

  } else {
    if (!params.contains("min_weight")) throw SRE("RISP: Need parameter min_weight.");
    if (!params.contains("max_weight")) throw SRE("RISP: Need parameter max_weight.");
    if (params.contains("inputs_from_weights")) {
      throw SRE("RISP: If you don't specify weights, you cannot specify inputs_from_weights.");
    }
    min_weight = params["min_weight"];
    max_weight = params["max_weight"];
  }

  if (params.contains("threshold_inclusive")) threshold_inclusive = params["threshold_inclusive"];

  /* Spike_value_factor has to be set after discrete and threshold_inclusive have been set. */

  if (params.contains("spike_value_factor")) {
    spike_value_factor = params["spike_value_factor"];

  /* If there are weights, and spike_value_factor is unspecified, then we're using
     inputs_from_weights, and spike_value_factor is meaningless. I'm going to set it
     to a big number to help debug. */

  } else if (weights.size() > 0) {
    spike_value_factor = -99999999.99;

  /* Otherwise, the default is max_weight.  
     If this is < max_threshold, then print a warning message on stderr. */

  } else {
    spike_value_factor = max_weight;
    if (max_weight < max_threshold || (!threshold_inclusive && max_weight == max_threshold)) {
      fprintf(stderr, "Warning: max_weight <%s max_threshold and spike_value_factor unset.\n",
                      (threshold_inclusive) ? "=" : "");
      fprintf(stderr, "Spike_value_factor set to %lg.\n", spike_value_factor);
    }
  } 
  
  if (params.contains("run_time_inclusive")) run_time_inclusive = params["run_time_inclusive"];
  if (params.contains("fire_like_ravens")) fire_like_ravens = params["fire_like_ravens"];
  if (params.contains("noisy_seed")) noisy_seed = params["noisy_seed"];
  if (params.contains("leak_mode")) leak_mode = params["leak_mode"];

  if (params.contains("stds")) stds = params["stds"].get< vector <double> >(); 
  if (params.contains("noisy_stddev")) noisy_stddev = params["noisy_stddev"]; 

  if (leak_mode != "all" && leak_mode != "none" && leak_mode != "configurable") {
    throw SRE("Reading processor json - bad leak_mode.  Must be all, none or configurable");
  }

  /* General Error Checking */

  if (discrete) {
    if (!is_integer(min_weight)) throw SRE("When discrete = true, min_weight must be an integer.");
    if (!is_integer(max_weight)) throw SRE("When discrete = true, max_weight must be an integer.");
    if (!is_integer(min_potential)) {
      throw SRE("When discrete = true, min_potential must be an integer.");
    }
    if (!is_integer(min_threshold)) {
      throw SRE("When discrete = true, min_threshold must be an integer.");
    }
    if (!is_integer(max_threshold)) {
      throw SRE("When discrete = true, max_threshold must be an integer.");
    }
    for (i = 0; i < weights.size(); i++) {
      if (!is_integer(weights[i])) throw SRE("When discrete = true, all weights must be integers.");
    }
  }
  if (min_potential > 0) throw SRE("Reading processor json - min_potential must be <= 0");

  if (stds.size() != 0 && weights.size() == 0) {
    throw SRE("If you specify stds, then you must specify weights and they must be the same size.");
  }

  if (stds.size() != 0 && weights.size() != stds.size()) {
    throw SRE("weights and stds must be the same size.");
  }

  if (stds.size() != 0 && noisy_stddev != 0) {
    throw SRE("Cannot specify both noisy_stddev and stds.");
  }

  if (stds.size() != 0 && discrete) throw SRE("Cannot specify stds and discrete = true.");
  if (noisy_stddev != 0 && discrete) throw SRE("Cannot specify noisy_stddev and discrete = true.");

  /* Have the saved parameters include all of the default information.   The reason is
     that this way, if defaults change, you can still have this information stored. */

  if (weights.size() == 0) {
    saved_params["min_weight"] = min_weight;
    saved_params["max_weight"] = max_weight;
    saved_params["spike_value_factor"] = spike_value_factor;
  } else {
    saved_params["weights"] = weights;
    saved_params["inputs_from_weights"] = inputs_from_weights;
    if (!inputs_from_weights) saved_params["spike_value_factor"] = spike_value_factor;
  }

  saved_params["max_delay"] = max_delay;
  saved_params["min_threshold"] = min_threshold;
  saved_params["max_threshold"] = max_threshold;
  saved_params["min_potential"] = min_potential;
  saved_params["discrete"] = discrete;

  saved_params["leak_mode"] = leak_mode;
  saved_params["fire_like_ravens"] = fire_like_ravens;
  saved_params["run_time_inclusive"] = run_time_inclusive;
  saved_params["threshold_inclusive"] = threshold_inclusive;
  if (noisy_seed != 0) saved_params["noisy_seed"] = noisy_seed;
  if (noisy_stddev != 0) saved_params["noisy_stddev"] = noisy_stddev;
  if (stds.size() != 0) saved_params["stds"] = stds;

};

Processor::~Processor(){
  map <int, risp::Network*>::const_iterator it;
  for (it = networks.begin(); it != networks.end(); ++it) delete it->second;
}

  /* Set the max spiking value (the value that corresponds to 1
     when you call apply_spikes() to either the max threshold, or
     if discrete&&!threshold_inclusive), then max threshold+1. */

bool Processor::load_network(neuro::Network* net, int network_id) {

  risp::Network *risp_net;
  string error;
  string rln = "risp::load_network() - ";

  /* Error Check properties */
  error = "";
  if (!net->is_node_property("Threshold")) error = rln + "Missing node' Threshold property\n";
  if (!net->is_edge_property("Weight")) error += (rln + "Missing edge Weight property\n");
  if (!net->is_edge_property("Delay")) error += (rln + "Missing edge Delay property\n");
  if (leak_mode[0] == 'c' && !net->is_node_property("Leak")) {
    error += (rln + "Missing node' Leak property\n");
  }

  if (net->get_properties().as_json() != get_network_properties().as_json()) {
    error += (rln + "neuro::Network's properties are");
    error += " different than processor's network properties\n";
  }

  if (error != "") {
    cerr << error;
    return false;
  }

  if (networks.find(network_id) != networks.end()) delete networks[network_id];

  risp_net = new risp::Network(net, 
                               spike_value_factor,
                               min_potential,
                               leak_mode[0], 
                               run_time_inclusive, 
                               threshold_inclusive, 
                               fire_like_ravens, 
                               discrete, 
                               inputs_from_weights,
                               noisy_seed, 
                               noisy_stddev,
                               weights,
                               stds);
  networks[network_id] = risp_net;

  return true;
}

bool Processor::load_networks(std::vector<neuro::Network*> &n) {
  size_t i, j;

  for (i = 0; i < n.size(); i++) {
    if(load_network(n[i], i) == false) {
      for (j = 0; j <= i; j++) {
        delete networks[j];
        networks.erase(j);
      }
      return false;
    }
  }
  return true;
}

void Processor::clear(int network_id) {
  risp::Network *risp_net = get_risp_network(network_id);
  networks.erase(network_id);
  delete risp_net;
}
  
 

void Processor::apply_spike(const Spike& s, int network_id) {
  get_risp_network(network_id)->apply_spike(s);
}



void Processor::apply_spike(const Spike& s, const vector<int>& network_ids) {
  size_t i;
  for (i = 0; i < network_ids.size(); i++) {
    apply_spike(s, network_ids[i]);
  }
}

void Processor::apply_spikes(const vector<Spike>& s, int network_id) {
  size_t i;
  risp::Network *risp_net = get_risp_network(network_id);

  for (i = 0; i < s.size(); i++) {
    risp_net->apply_spike(s[i]);
  }
}


void Processor::apply_spikes(const vector<Spike>& s, const vector<int>& network_ids) {
  size_t i;
  for (i = 0; i < network_ids.size(); i++) {
    apply_spikes(s, network_ids[i]);
  }
}

  
void Processor::run(double duration, int network_id) {
  get_risp_network(network_id)->run(duration);
}

void Processor::run(double duration, const vector<int>& network_ids) {
  size_t i;
  for (i = 0; i < network_ids.size(); i++) {
    get_risp_network(network_ids[i])->run(duration);
  }
}

 
long long Processor::total_neuron_counts(int network_id) {
  return get_risp_network(network_id)->total_neuron_counts();
}

long long Processor::total_neuron_accumulates(int network_id) {
  return get_risp_network(network_id)->total_neuron_accumulates();
}

double Processor::get_time(int network_id) {
  return get_risp_network(network_id)->get_time();
}

bool Processor::track_output_events(int output_id, bool track, int network_id) {
  return get_risp_network(network_id)->track_output_events(output_id, track);
}

bool Processor::track_neuron_events(uint32_t node_id, bool track, int network_id) {
  return get_risp_network(network_id)->track_neuron_events(node_id, track);
}

double Processor::output_last_fire(int output_id, int network_id) {
  return get_risp_network(network_id)->output_last_fire(output_id);
}

vector <double> Processor::output_last_fires(int network_id) {
  return get_risp_network(network_id)->output_last_fires();
}

int Processor::output_count(int output_id, int network_id) {
  return get_risp_network(network_id)->output_count(output_id);
}

vector <int> Processor::output_counts(int network_id) {
  return get_risp_network(network_id)->output_counts();
}

vector <double> Processor::output_vector(int output_id, int network_id) {
  return get_risp_network(network_id)->output_vector(output_id);
}

vector < vector <double> > Processor::output_vectors(int network_id) {
  return get_risp_network(network_id)->output_vectors();
}


vector <int> Processor::neuron_counts(int network_id) {
  return get_risp_network(network_id)->neuron_counts();
}

vector < vector <double> > Processor::neuron_vectors(int network_id) {
  return get_risp_network(network_id)->neuron_vectors();
}
vector < double > Processor::neuron_charges(int network_id) {
  return get_risp_network(network_id)->neuron_charges();
}

vector <double> Processor::neuron_last_fires(int network_id) {
  return get_risp_network(network_id)->neuron_last_fires();
}

void  Processor::synapse_weights(vector <uint32_t> &pres,
                               vector <uint32_t> &posts,
                               vector <double> &vals,
                               int network_id)
{
  return get_risp_network(network_id)->synapse_weights(pres, posts, vals);
}


/* Remove state, keep network loaded */
void Processor::clear_activity(int network_id) {
  get_risp_network(network_id)->clear_activity();
}

PropertyPack Processor::get_network_properties() const 
{
  PropertyPack pp;

  pp.add_node_property("Threshold", min_threshold, max_threshold, 
                       (discrete) ? Property::Type::INTEGER : Property::Type::DOUBLE);

  if (leak_mode[0] == 'c') pp.add_node_property("Leak", 0, 1, Property::Type::BOOLEAN);

  if (weights.size() > 0) {
    pp.add_edge_property("Weight", 0, weights.size()-1, Property::Type::INTEGER);
  } else {
    pp.add_edge_property("Weight", min_weight, max_weight, 
                        (discrete) ? Property::Type::INTEGER : Property::Type::DOUBLE);
  }
  pp.add_edge_property("Delay", min_delay, max_delay, Property::Type::INTEGER);

  return pp;
}

json Processor::get_processor_properties() const {
  
  json j = json::object();

  j["spike_value_factor"] = spike_value_factor;
  j["binary_input"] = false;
  j["spike_raster_info"] = true;
  j["plasticity"] = "none";
  j["run_time_inclusive"] = run_time_inclusive;
  j["integration_delay"] = false;
  j["threshold_inclusive"] = threshold_inclusive;

  return j;
}

json Processor::get_params() const {
  return saved_params;
}

string Processor::get_name() const {
  return "risp";
}

Network* Processor::get_risp_network(int network_id) 
 {
  map <int, risp::Network*>::const_iterator it;
  char buf[200];
  it = networks.find(network_id);
  if (it == networks.end()) {
    snprintf(buf, 200, "risp::Processor::get_risp_network() network_id %d does not exist", 
             network_id);
    throw SRE((string) buf);
  }
  return it->second;
}

}

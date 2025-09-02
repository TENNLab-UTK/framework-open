#include <iostream>
#include "izhikevich.hpp"
#include "utils/json_helpers.hpp"
using SRE = std::runtime_error;
using namespace std;

nlohmann::json izhikevich::Processor::spec = {
  { "min_v", "D" },
  { "max_v", "D" },
  { "min_u", "D" },
  { "max_u", "D" },
  { "min_a", "D" },
  { "max_a", "D" },
  { "min_b", "D" },
  { "max_b", "D" },
  { "min_c", "D" },
  { "max_c", "D" },
  { "min_d", "D" },
  { "max_d", "D" },

  { "min_synaptic_current", "D" },
  { "max_synaptic_current", "D" },
  { "min_synaptic_delay",   "I" },
  { "max_synaptic_delay",   "I" },

  { "min_input_current",         "D" },
  { "max_input_current",         "D" },
  { "milliseconds_per_timestep", "D" }
};

izhikevich::Neuron::Neuron(
  neuro::Node* n,
  const Processor* p,
  int index)
  : tau(p->tau)
{
    if (!n) {
        throw SRE(
          "izhikevich::Neuron::Neuron(): "
          "bad node argument: invalid pointer");
    }

    node = n;
    this->index = index;

    tracking = false;
    last_fire = -1;
    fire_count = 0;

    v = n->get("v");
    u = n->get("u");
    a = n->get("a");
    b = n->get("b");
    c = n->get("c");
    d = n->get("d");
    I = 0;
}

izhikevich::Neuron::~Neuron()
{
    size_t i;

    for (i = 0; i < synapses.size(); ++i) {
        delete synapses[i];
    }
}

void izhikevich::Neuron::set_network(Network* network)
{
    net = network;
}

void izhikevich::Neuron::update()
{
    v += tau * (0.04 * v * v + 5 * v + 140 - u + I);
    u += tau * a * (b * v - u);
}

bool izhikevich::Neuron::has_to_fire() const
{
    return v >= 30;
}

void izhikevich::Neuron::fire()
{
    size_t i;
    int time, to;

    for (i = 0; i < synapses.size(); ++i) {
        time = net->timestep + synapses[i]->delay;
        to = synapses[i]->to->index;
        net->events[time][to] += synapses[i]->current;
    }

    v = c;
    u += d;
    fired = true;
    last_fire = net->timestep;
    fire_count++;
    if (tracking) {
        fire_times.push_back(net->timestep);
    }
}

void izhikevich::Neuron::reset_input_current()
{
    I = 0;
}

void izhikevich::Neuron::reset_firing_info()
{
    fired = false;
    last_fire = -1;
    fire_count = 0;
    fire_times.clear();
}

void izhikevich::Neuron::reset_state()
{
    v = node->get("v");
    u = node->get("u");
    reset_input_current();
    reset_firing_info();
}

izhikevich::Synapse::Synapse(
  Neuron* from,
  Neuron* to,
  neuro::Edge* e)
{
    if (!e) {
        throw SRE(
          "izhikevich::Synapse::Synapse(): "
          "bad edge argument: invalid pointer");
    }

    this->from = from;
    this->to = to;
    current = e->get("current");
    delay = e->get("delay");
}

izhikevich::Network::Network(
  neuro::Network* net,
  const Processor* p)
{
    int to;
    size_t i, j;
    neuro::Node* node;
    neuro::Edge* edge;
    Synapse* syn;

    try {
        net->make_sorted_node_vector();
        neurons.resize(net->num_nodes());
        for (i = 0; i < net->sorted_node_vector.size(); ++i) {
            node = net->sorted_node_vector[i];
            indices[node->id] = i;
            neurons[i] = new Neuron(node, p, i);
            if (node->is_input()) inputs.push_back(i);
            if (node->is_output()) outputs.push_back(i);
        }
    
        nsynapses = 0;
        for (i = 0; i < net->sorted_node_vector.size(); ++i) {
            node = net->sorted_node_vector[i];
            for (j = 0; j < node->outgoing.size(); ++j) {
                edge = node->outgoing[j];
                to = indices[edge->to->id];
                syn = new Synapse(neurons[i], neurons[to], edge);
                neurons[i]->synapses.push_back(syn);
            }
            nsynapses += neurons[i]->synapses.size();
        }
    
        timestep = 0;
        total_fires = 0;
        total_accumulates = 0;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

izhikevich::Network::~Network()
{
    size_t i;

    for (i = 0; i < neurons.size(); ++i) {
        delete neurons[i];
    }
}

izhikevich::Neuron* izhikevich::Network::get_neuron(
  uint32_t node_id) const
{
    char buf[20];
    unordered_map<uint32_t, int>::const_iterator it;

    it = indices.find(node_id);
    if (it == indices.end()) {
        snprintf(buf, 20, "%u", node_id);
        throw SRE(
          "izhikevich::Network::get_neuron(): "
          "bad node id: " + string(buf) + ": "
          "node id does not match that of any existing node");
    }
    return neurons[it->second];
}

izhikevich::Neuron* izhikevich::Network::get_output(
  int output_id) const
{
    char buf[20];
    string s;

    if (output_id < 0) {
        s = "output id must be greater than or equal to zero";
    } else if ((size_t)output_id >= outputs.size()) {
        s = "output id does not match that of any existing output "
          "node";
    }

    if (!s.empty()) {
        snprintf(buf, 20, "%d", output_id);
        throw SRE(
          "izhikevich::Network::get_output(): "
          "bad output id: " + string(buf) + ": " + s);
    }
    return neurons[outputs[output_id]];
}

izhikevich::Processor::Processor(nlohmann::json& arg)
{
    bool valid;
    char buf[100];
    size_t i;
    string l, r;
    string min, max, which;
    vector<string> names;

    try {
        neuro::Parameter_Check_Json_T(arg, spec);

        params["min_v"] = -65;
        params["max_v"] = -65;
        params["min_u"] = -13;
        params["max_u"] = -13;
        params["min_a"] = 0.02;
        params["max_a"] = 0.02;
        params["min_b"] = 0.2;
        params["max_b"] = 0.2;
        params["min_c"] = -65;
        params["max_c"] = -65;
        params["min_d"] = 8;
        params["max_d"] = 8;

        params["min_synaptic_current"] = 0;
        params["max_synaptic_current"] = 0.5;
        params["min_synaptic_delay"] = 1;
        params["max_synaptic_delay"] = 1;

        params["min_input_current"] = -15;
        params["max_input_current"] = 15;
        params["milliseconds_per_timestep"] = 1;

        names.push_back("min_synaptic_delay");
        names.push_back("max_synaptic_delay");
        names.push_back("milliseconds_per_timestep");

        for (i = 0; i < names.size(); ++i) {
            if (arg.contains(names[i])) {
                if (names[i] == "milliseconds_per_timestep") {
                    if (arg[names[i]] < 0) {
                        throw 1;
                    } else {
                        params[names[i]] = arg[names[i]];
                    }
                } else if (!arg[names[i]].is_number_integer()) {
                    throw 2;
                } else if (arg[names[i]] < 1) {
                    throw 3;
                }
            }
        }
        names.clear();

        names.push_back("v");
        names.push_back("u");
        names.push_back("a");
        names.push_back("b");
        names.push_back("c");
        names.push_back("d");
        names.push_back("synaptic_current");
        names.push_back("synaptic_delay");
        names.push_back("input_current");

        for (i = 0; i < names.size(); ++i) {
            snprintf(buf, 100, "min_%s", names[i].c_str());
            min = buf;
            snprintf(buf, 100, "max_%s", names[i].c_str());
            max = buf;

            valid = false;
            if (arg.contains(min) && arg.contains(max)) {
                if (arg[min] > arg[max]) {
                    l = "provided";
                    r = "provided";
                } else {
                    valid = true;
                }
            } else if (arg.contains(min) && arg[min] > params[max]) {
                l = "provided";
                r = "default";
            } else if (arg.contains(max) && params[min] > arg[max]) {
                l = "default";
                r = "provided";
            } else {
                valid = true;
            }

            if (!valid) {
                which = (arg.contains(min)) ? min : max;
                throw 4;
            }

            if (arg.contains(min)) params[min] = arg[min];
            if (arg.contains(max)) params[max] = arg[max];
        }

        min_input_current = params["min_input_current"];
        max_input_current = params["max_input_current"];
        milliseconds_per_timestep =
          params["milliseconds_per_timestep"];
        tau = milliseconds_per_timestep;

        properties = nlohmann::json::object();
        properties["threshold_inclusive"] = true;
        properties["input_scaling_value"] = -1;
        properties["binary_input"] = false;
        properties["spike_raster_info"] = true;
        properties["plasticity"] = "none";
        properties["run_time_inclusive"] = false;
        properties["integration_delay"] = false;

        ppack.add_node_property(
          "v",
          params["min_v"],
          params["max_v"],
          neuro::Property::Type::DOUBLE);
        ppack.add_node_property(
          "u",
          params["min_u"],
          params["max_u"],
          neuro::Property::Type::DOUBLE);
        ppack.add_node_property(
          "a",
          params["min_a"],
          params["max_a"],
          neuro::Property::Type::DOUBLE);
        ppack.add_node_property(
          "b",
          params["min_b"],
          params["max_b"],
          neuro::Property::Type::DOUBLE);
        ppack.add_node_property(
          "c",
          params["min_c"],
          params["max_c"],
          neuro::Property::Type::DOUBLE);
        ppack.add_node_property(
          "d",
          params["min_d"],
          params["max_d"],
          neuro::Property::Type::DOUBLE);

        ppack.add_edge_property(
          "current",
          params["min_synaptic_current"],
          params["max_synaptic_current"],
          neuro::Property::Type::DOUBLE);
        ppack.add_edge_property(
          "delay",
          params["min_synaptic_delay"],
          params["max_synaptic_delay"],
          neuro::Property::Type::INTEGER);
    } catch (const int e) {
        switch (e) {
        case 1: throw SRE(
                  "izhikevich::Processor::Processor(): "
                  "bad parameter: \"" + names[i] + "\": "
                  "parameter must be greater than zero");
        case 2: throw SRE(
                  "izhikevich::Processor::Processor(): "
                  "bad parameter: \"" + names[i] + "\": "
                  "parameter must be an integer");
        case 3: throw SRE(
                  "izhikevich::Processor::Processor(): "
                  "bad parameter: \"" + names[i] + "\": "
                  "parameter must be greater than or equal to one");
        case 4: throw SRE(
                  "izhikevich::Processor::Processor(): "
                  "bad parameter: \"" + which + "\": " +
                  l + " \"" + min + "\" greater than " +
                  r + " \"" + max + "\"");
        default: throw SRE(
                   "izhikevich::Processor::Processor(): "
                   "unknown error");
        }
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

izhikevich::Processor::~Processor()
{
    unordered_map<int, Network*>::iterator it;

    for (it = networks.begin(); it != networks.end(); ++it) {
        delete it->second;
    }
}

bool izhikevich::Processor::load_network(
  neuro::Network* network,
  int network_id)
{
    size_t i;
    Network* net;

    try {
        if (!network) {
            throw SRE(
              "izhikevich::Processor::load_network(): "
              "bad network argument: "
              "invalid pointer");
        }
        if (network->get_properties().as_json() != ppack.as_json()) {
            throw SRE(
              "izhikevich::Processor::load_network(): "
              "bad network argument: "
              "the network's PropertyPack differs from the "
              "PropertyPack of the neuroprocessor");
        }
        // if (network->get_data("proc_params") != params) {
        //     throw SRE(
        //       "izhikevich::Processor::load_network(): "
        //       "bad network argument: "
        //       "the parameters in \"proc_params\" in the network's "
        //       "\"Associated_Data\" differ from the parameters of "
        //       "the neuroprocessor");
        // }
        if (network->get_data("other")["proc_name"] != "izhikevich") {
            throw SRE(
              "izhikevich::Processor::load_network(): "
              "bad network argument: "
              "\"proc_name\" in the network's \"Associated_Data\" "
              "not \"izhikevich\"");
        }

        clear(network_id);
        net = new Network(network, this);
        for (i = 0; i < net->neurons.size(); ++i) {
            net->neurons[i]->set_network(net);
        }
        networks[network_id] = net;
        return true;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

bool izhikevich::Processor::load_networks(
  vector<neuro::Network*>& networks)
{
    size_t i;

    for (i = 0; i < networks.size(); ++i) {
        load_network(networks[i], i);
    }

    return true;
}

void izhikevich::Processor::clear(int network_id)
{
    unordered_map<int, Network*>::iterator it;

    it = networks.find(network_id);

    if (it != networks.end()) {
        delete it->second;
        networks.erase(it);
    }
}

void izhikevich::Processor::apply_spike(
    const neuro::Spike& spike,
    bool normalized,
    int network_id)
{
    char buf[20];
    int time, index;
    double charge;
    string s;
    Network* net;

    try {
        net = get_network(network_id);

        if (spike.id < 0) {
            s = "input id must be greater than or equal to zero";
        } else if ((size_t)spike.id >= net->inputs.size()) {
            s = "input id does not match that of any existing input "
              "node";
        }

        if (!s.empty()) {
            snprintf(buf, 20, "%d", spike.id);
            throw SRE(
              "izhikevich::Processor::apply_spike(): "
              "bad input id: " + string(buf) + ": " + s);
        }

        if (spike.time < 0) {
            snprintf(buf, 20, "%lf", spike.time);
            throw SRE(
              "izhikevich::Processor::apply_spike(): "
              "bad spike time: " + string(buf) + ": "
              "spike time must be greater than or equal to zero");
        }

        if (normalized) {
            if (spike.value < 0 || spike.value > 1) {
                snprintf(buf, 20, "%lf", spike.value);
                throw SRE(
                  "izhikevich::Processor::apply_spike(): "
                  "bad spike value: " + string(buf) + ": "
                  "spike value must lie on the interval [-1, 1] "
                  "when 'normalized' is set to true");
            }
            charge = spike.value * (max_input_current -
              min_input_current) + min_input_current;
        } else {
            charge = spike.value;
        }
        time = net->timestep + spike.time;
        index = net->inputs[spike.id];
        net->events[time][index] += charge;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

void izhikevich::Processor::apply_spike(
  const neuro::Spike& spike,
  const vector<int>& network_ids,
  bool normalized)
{
    size_t i;

    for (i = 0; i < network_ids.size(); ++i) {
        apply_spike(spike, normalized, network_ids[i]);
    }
}

void izhikevich::Processor::apply_spikes(
  const vector<neuro::Spike>& spikes,
  bool normalized,
  int network_id)
{
    size_t i;

    for (i = 0; i < spikes.size(); ++i) {
        apply_spike(spikes[i], normalized, network_id);
    }
}

void izhikevich::Processor::apply_spikes(
  const vector<neuro::Spike>& spikes,
  const vector<int>& network_ids,
  bool normalized)
{
    size_t i;

    for (i = 0; i < spikes.size(); ++i) {
        apply_spike(spikes[i], network_ids, normalized);
    }
}

void izhikevich::Processor::run(
  double duration,
  int network_id)
{
    char buf[20];
    int index;
    size_t i, j;
    double charge;
    Neuron* n;
    Network* net;
    unordered_map<int, double>::iterator it, beg, end;

    if (duration == 0) {
        return;
    }

    try {
        if (duration < 0) {
            snprintf(buf, 20, "%lf", duration);
            throw SRE(
              "izhikevich::Processor::run(): "
              "bad duration: " + string(buf) + ": "
              "duration must be greater than or equal to zero");
        }

        net = get_network(network_id);
        for (i = 0; i < net->neurons.size(); ++i) {
            net->neurons[i]->reset_firing_info();
        }

        for (i = 0; i < duration; ++i) {
            beg = net->events[net->timestep].begin();
            end = net->events[net->timestep].end();
            for (it = beg; it != end; ++it) {
                index = it->first;
                charge = it->second;
                net->neurons[index]->I += charge;
            }
            net->events.erase(net->timestep);

            for (j = 0; j < net->neurons.size(); ++j) {
                n = net->neurons[j];
                n->update();
                if (n->has_to_fire()) {
                    n->fire();
                    net->total_fires++;
                    net->total_accumulates += n->synapses.size();
                }
                n->reset_input_current();
            }
            net->timestep++;
        }
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

void izhikevich::Processor::run(
  double duration,
  const vector<int>& network_ids)
{
    size_t i;

    for (i = 0; i < network_ids.size(); ++i) {
        run(duration, network_ids[i]);
    }
}

double izhikevich::Processor::get_time(int network_id)
{
    try {
        return get_network(network_id)->timestep;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

bool izhikevich::Processor::track_output_events(
  int output_id,
  bool track,
  int network_id)
{
    Neuron* out;
    Network* net;

    try {
        net = get_network(network_id);
        out = net->get_output(output_id);
        out->tracking = track;
        return true;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

bool izhikevich::Processor::track_neuron_events(
  uint32_t node_id,
  bool track,
  int network_id)
{
    Neuron* n;
    Network* net;

    try {
        net = get_network(network_id);
        n = net->get_neuron(node_id);
        n->tracking = track;
        return true;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

double izhikevich::Processor::output_last_fire(
  int output_id,
  int network_id)
{
    Neuron* out;
    Network* net;

    try {
        net = get_network(network_id);
        out = net->get_output(output_id);
        return out->last_fire;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

vector<double> izhikevich::Processor::output_last_fires(int network_id)
{
    size_t i;
    int index;
    Network* net;
    vector<double> times;

    try {
        net = get_network(network_id);
        times.resize(net->outputs.size());
        for (i = 0; i < net->outputs.size(); ++i) {
            index = net->outputs[i];
            times[i] = net->neurons[index]->last_fire;
        }
        return times;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

int izhikevich::Processor::output_count(
  int output_id,
  int network_id)
{
    Neuron* out;
    Network* net;

    try {
        net = get_network(network_id);
        out = net->get_output(output_id);
        return out->fire_count;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

vector<int> izhikevich::Processor::output_counts(int network_id)
{
    size_t i;
    int index;
    Network* net;
    vector<int> counts;

    try {
        net = get_network(network_id);
        counts.resize(net->outputs.size());
        for (i = 0; i < net->outputs.size(); ++i) {
            index = net->outputs[i];
            counts[i] = net->neurons[index]->fire_count;
        }
        return counts;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

vector<double> izhikevich::Processor::output_vector(
  int output_id,
  int network_id)
{
    Neuron* out;
    Network* net;

    try {
        net = get_network(network_id);
        out = net->get_output(output_id);
        return out->fire_times;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

vector<vector<double>> izhikevich::Processor::output_vectors(
  int network_id)
{
    size_t i;
    int index;
    Network* net;
    vector<vector<double>> times;

    try {
        net = get_network(network_id);
        times.resize(net->outputs.size());
        for (i = 0; i < net->outputs.size(); ++i) {
            index = net->outputs[i];
            times[i] = net->neurons[index]->fire_times;
        }
        return times;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

long long izhikevich::Processor::total_neuron_counts(int network_id)
{
    long long count;
    Network* net;

    try {
        net = get_network(network_id);
        count = net->total_fires;
        net->total_fires = 0;
        return count;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

long long izhikevich::Processor::total_neuron_accumulates(
  int network_id)
{
    long long count;
    Network* net;

    try {
        net = get_network(network_id);
        count = net->total_accumulates;
        net->total_accumulates = 0;
        return count;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

vector<int> izhikevich::Processor::neuron_counts(int network_id)
{
    size_t i;
    Network* net;
    vector<int> counts;

    try {
        net = get_network(network_id);
        counts.resize(net->neurons.size());
        for (i = 0; i < net->neurons.size(); ++i) {
            counts[i] = net->neurons[i]->fire_count;
        }
        return counts;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

vector<double> izhikevich::Processor::neuron_last_fires(int network_id)
{
    size_t i;
    Network* net;
    vector<double> times;

    try {
        net = get_network(network_id);
        times.resize(net->neurons.size());
        for (i = 0; i < net->neurons.size(); ++i) {
            times[i] = net->neurons[i]->last_fire;
        }
        return times;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

vector<vector<double>> izhikevich::Processor::neuron_vectors(
  int network_id)
{
    size_t i;
    Network* net;
    vector<vector<double>> times;

    try {
        net = get_network(network_id);
        times.resize(net->neurons.size());
        for (i = 0; i < net->neurons.size(); ++i) {
            times[i] = net->neurons[i]->fire_times;
        }
        return times;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

vector<double> izhikevich::Processor::neuron_charges(int network_id)
{
    size_t i;
    Neuron* n;
    Network* net;
    vector<double> charges;

    try {
        net = get_network(network_id);
        charges.resize(net->neurons.size());
        for (i = 0; i < net->neurons.size(); ++i) {
            n = net->neurons[i];
            charges[i] = (n->fired) ? 30 : n->v;
        }
        return charges;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

void izhikevich::Processor::synapse_weights(
  vector<uint32_t>& pres,
  vector<uint32_t>& posts,
  vector<double>& vals,
  int network_id)
{
    size_t i, j, k;
    Neuron* n;
    Network* net;

    try {
        net = get_network(network_id);
        pres.resize(net->nsynapses);
        posts.resize(net->nsynapses);
        vals.resize(net->nsynapses);
        k = 0;
        for (i = 0; i < net->neurons.size(); ++i) {
            n = net->neurons[i];
            for (j = 0; j < n->synapses.size(); ++j) {
                pres[k] = n->node->id;
                posts[k] = n->synapses[j]->to->node->id;
                vals[k] = n->synapses[j]->current;
                ++k;
            }
        }
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

void izhikevich::Processor::clear_activity(int network_id)
{
    size_t i;
    Network* net;

    try {
        net = get_network(network_id);
        for (i = 0; i < net->neurons.size(); ++i) {
            net->neurons[i]->reset_state();
        }
        net->events.clear();
        net->timestep = 0;
    } catch (std::exception& e) {
        cerr << e.what() << '\n';
        exit(1);
    }
}

neuro::PropertyPack
  izhikevich::Processor::get_network_properties() const
{
    return ppack;
}

nlohmann::json izhikevich::Processor::get_processor_properties() const
{
    return properties;
}

nlohmann::json izhikevich::Processor::get_params() const
{
    return params;
}

string izhikevich::Processor::get_name() const
{
    return "izhikevich";
}

izhikevich::Network* izhikevich::Processor::get_network(
  int network_id) const
{
    char buf[20];
    unordered_map<int, Network*>::const_iterator it;

    it = networks.find(network_id);
    if (it == networks.end()) {
        snprintf(buf, 20, "%d", network_id);
        throw SRE(
          "izhikevich::Processor::get_network(): "
          "bad network id: " + string(buf) + ": "
          "network id does not match that of any existing network");
    }
    return it->second;
}

/* Helper routines to facilitate apps and help processor writers keep it simple.
 */

#include "framework.hpp"
#include "utils/json_helpers.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>

typedef std::runtime_error SRE;

namespace neuro {
using nlohmann::json;
using std::make_pair;
using std::string;
using std::vector;

/* Iterate and call track_output_events() for every output neuron in the
   network. If this fails, "undo" all of the previous calls. */

bool track_all_output_events(Processor* p, Network* n, int network_id) {
    int num;

    try {
        for (num = 0; num < n->num_outputs(); num++) {
            if (!p->track_output_events(num, true, network_id))
                throw "";
        }
    } catch (...) {
        while (num > 0) {
            num--;
            (void)p->track_output_events(num, false, network_id);
        }
        return false;
    }

    return true;
}

/* Iterate and call track_neuron_events() for every neuron in the network.
   If this fails, we're simply returning false, and not trying to undo anything.
 */

bool track_all_neuron_events(Processor* p, Network* n, int network_id) {
    NodeMap::iterator nit;

    for (nit = n->begin(); nit != n->end(); nit++) {
        if (!p->track_neuron_events(nit->second->id, true, network_id))
            return false;
    }

    return true;
}

/* If counts.size() == 0, then we assume that the processor does not implement
   neuron counts.  Instead of flagging an error, we simply create the proper
   JSON with no neuron counts. */

json neuron_counts_to_json(const vector<int>& counts, Network* n) {
    size_t i;
    vector<uint32_t> neurons;
    vector<int> tc;
    json rv;

    n->make_sorted_node_vector();
    if (counts.size() != 0 && counts.size() != n->sorted_node_vector.size()) {
        throw SRE((string) "neuron_counts_to_json: counts vector size != # "
                           "neurons in network.");
    }
    for (i = 0; i < n->sorted_node_vector.size(); i++) {
        neurons.push_back(n->sorted_node_vector[i]->id);
    }
    rv = json::object();
    rv["Neuron Alias"] = neurons;
    if (counts.size() == 0) {
        tc.resize(neurons.size(), 0);
        rv["Event Counts"] = tc;
    } else {
        rv["Event Counts"] = counts;
    }
    return rv;
}

/* If counts.size() == 0, then we assume that the processor does not implement
   neuron counts.  Instead of flagging an error, we simply create the proper
   JSON with no neuron counts. */

json neuron_charges_to_json(const vector<double>& charges, Network* n) {
    size_t i;
    vector<uint32_t> neurons;
    vector<double> tc;
    json rv;

    n->make_sorted_node_vector();
    if (charges.size() != 0 && charges.size() != n->sorted_node_vector.size()) {
        throw SRE((string) "neuron_charges_to_json: charges vector size != # "
                           "neurons in network.");
    }
    for (i = 0; i < n->sorted_node_vector.size(); i++) {
        neurons.push_back(n->sorted_node_vector[i]->id);
    }
    rv = json::object();
    rv["Neuron Alias"] = neurons;
    if (charges.size() == 0) {
        tc.resize(neurons.size(), 0);
        rv["Charges"] = tc;
    } else {
        rv["Charges"] = charges;
    }
    return rv;
}

/* If last_fires.size() == 0, then we assume that the processor does not
   implement neuron last fires.  Instead of flagging an error, we simply create
   the proper JSON where all of the last firing times are -1. */

json neuron_last_fires_to_json(const vector<double>& last_fires, Network* n) {
    size_t i;
    vector<uint32_t> neurons;
    vector<int> tlf;
    json rv;

    n->make_sorted_node_vector();
    if (last_fires.size() != 0 &&
        last_fires.size() != n->sorted_node_vector.size()) {
        throw SRE((string) "neuron_last_fires_to_json: last_fires vector size "
                           "!= # neurons in network.");
    }
    for (i = 0; i < n->sorted_node_vector.size(); i++) {
        neurons.push_back(n->sorted_node_vector[i]->id);
    }

    rv = json::object();
    rv["Neuron Alias"] = neurons;
    if (last_fires.size() == 0) {
        tlf.resize(neurons.size(), -1);
        rv["Last Fires"] = tlf;
    } else {
        rv["Last Fires"] = last_fires;
    }
    return rv;
}

/* If events.size() == 0, we assume that the processor doesn't implement
   events.  We simply create the JSON as if none of the neurons had any events.
 */

json neuron_vectors_to_json(const vector<vector<double>>& events,
                            const string& type, Network* n) {
    json rv;
    size_t i, j;
    int ssize, v;
    vector<string> spikes;
    vector<uint32_t> neurons;
    vector<vector<int>> te;

    n->make_sorted_node_vector();
    if (events.size() != 0 && events.size() != n->sorted_node_vector.size()) {
        throw SRE((string) "neuron_vectors_to_json: vectors size != # neurons "
                           "in network.");
    }
    for (i = 0; i < n->sorted_node_vector.size(); i++) {
        neurons.push_back(n->sorted_node_vector[i]->id);
    }

    rv = json::object();
    rv["Neuron Alias"] = neurons;
    switch (type[0]) {
    case 'V':
        if (events.size() == 0) {
            te.resize(neurons.size());
            rv["Event Times"] = te;
        } else {
            rv["Event Times"] = events;
        }
        break;

    case 'S': // Make spike strings of 0's and 1's.
        if (events.size() == 0) {
            spikes.resize(neurons.size(), "0");
        } else {
            ssize = -1;
            for (i = 0; i < events.size(); i++) {
                for (j = 0; j < events[i].size(); j++) {
                    v = (int)events[i][j];
                    if (v > ssize)
                        ssize = v;
                }
            }
            ssize++;
            spikes.resize(events.size());
            for (i = 0; i < events.size(); i++) {
                spikes[i].resize(ssize, '0');
                for (j = 0; j < events[i].size(); j++) {
                    v = (int)events[i][j];
                    spikes[i][v] = '1';
                }
            }
        }
        rv["Spikes"] = spikes;
        break;

    default:
        throw SRE((string) "neuron_vectors_to_json - bad type: " + type);
    }
    return rv;
}

Network* pull_network(Processor* p, Network* n, int network_id) {
    vector<uint32_t> pres;
    vector<uint32_t> posts;
    vector<double> vals;
    size_t i;
    char buf[100];
    const Property* prop;
    Edge* e;
    Network* nn;

    /* Grab the synapse weights */

    p->synapse_weights(pres, posts, vals, network_id);

    /* Error check to make sure the network has the neurons / synapses. */

    for (i = 0; i < pres.size(); i++) {
        if (!n->is_node(pres[i])) {
            snprintf(buf, 100,
                     "pull_network(): Synapse from %u - Node %u is not in the "
                     "network.",
                     pres[i], pres[i]);
            throw SRE((string)buf);
        }
        if (!n->is_node(posts[i])) {
            snprintf(buf, 100,
                     "pull_network(): Synapse to %u - Node %u is not in the "
                     "network.",
                     posts[i], posts[i]);
            throw SRE((string)buf);
        }

        if (!n->is_edge(pres[i], posts[i])) {
            snprintf(
                buf, 100,
                "pull_network(): Synapse from %u to %u is not in the network.",
                pres[i], posts[i]);
            throw SRE((string)buf);
        }
    }

    /* Make sure that the synapses have a "Weight" property */

    if (!n->is_edge_property("Weight")) {
        throw SRE("pull_network(): Synapses don't have a \"Weight\" property");
    }
    prop = n->get_edge_property("Weight");

    /* Copy the network and set synpases */

    nn = new Network(*n);
    for (i = 0; i < pres.size(); i++) {
        e = nn->get_edge(pres[i], posts[i]);
        e->values[prop->index] = vals[i];
    }
    return nn;
}

/* Do run(1) duration times, and keep track of spike_raster/charge info. */

json run_and_track(int duration, Processor* p, int network_id) {
    int i;
    size_t j;
    json rv;
    vector<double> charges;
    vector<int> counts;

    rv["spike_raster"] = json::array();
    rv["charges"] = json::array();

    for (i = 0; i < duration; i++) {
        p->run(1, network_id);
        counts = p->neuron_counts(network_id);
        charges = p->neuron_charges(network_id);
        rv["spike_raster"].push_back(json::array());
        rv["charges"].push_back(json::array());
        for (j = 0; j < counts.size(); j++) {
            rv["spike_raster"][i].push_back((counts[j] != 0) ? 1 : 0);
        }
        for (j = 0; j < charges.size(); j++)
            rv["charges"][i].push_back(charges[j]);
    }

    return rv;
}

void apply_spike_raster(Processor* p, int in_neuron, const vector<char>& sr,
                        int network_id) {
    size_t i;
    Spike s(in_neuron, 0, 1);

    for (i = 0; i < sr.size(); i++) {
        if (sr[i]) {
            s.time = i;
            p->apply_spike(s, true, network_id);
        }
    }
}

/* End of namespace */
} // namespace neuro

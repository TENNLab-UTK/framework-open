#pragma once

#include "framework.hpp"
#include "nlohmann/json.hpp"
#include "utils/MOA.hpp"
#include "utils/alignment_helpers.hpp"
#include <cstdint>
#include <map>

using namespace neuro;
using namespace std;
namespace vrisp {
class Network;
class Processor;

class Network {
  public:
    /** Convert network in framework format to an internal vrisp network */

    Network(neuro::Network* net, double _min_potential, char leak,
            size_t tracked_timesteps, double spike_value_factor);
    ~Network();

    /* Mirror calls from the Processor API */
    void apply_spike(const Spike& s, bool normalized = true);
    void run(size_t duration);
    double get_time();

    double output_last_fire(int output_id);
    vector<double> output_last_fires();

    int output_count(int output_id);
    vector<int> output_counts();

    vector<double> output_vector(int output_id);
    vector<vector<double>> output_vectors();

    long long total_neuron_counts();
    long long total_neuron_accumulates();
    vector<int> neuron_counts();
    vector<double> neuron_last_fires();
    vector<vector<double>> neuron_vectors();

    vector<double> neuron_charges();
    void synapse_weights(vector<uint32_t>& pres, vector<uint32_t>& posts,
                         vector<double>& vals);

    void clear_activity();
    void clear_output_tracking();

  protected:
    void process_events(uint32_t time); /**< Process events at time "time" */

    vector<bool> inputs;
    vector<bool> outputs;

    vector<size_t> input_mappings;
    vector<size_t> output_mappings;
    vector<size_t> neuron_mappings;

    size_t neuron_count;
    size_t allocation_size;
    size_t tracked_timesteps_count;

    vector<uint8_t, AlignmentAllocator<uint8_t>>
        neuron_fired; /**< Bitmap, Did this neuron fire on this timestep */
    vector<uint8_t, AlignmentAllocator<uint8_t>>
        output_fire_count; /**< Number of fires since last run() call*/
    vector<int8_t, AlignmentAllocator<int8_t>> output_last_fire_timestep;
    /**<
                 Timestep of last firing for this neuron */
    vector<int8_t, AlignmentAllocator<int8_t>>
        neuron_threshold; /**< Neuron's threshold*/
    vector<vector<uint16_t, AlignmentAllocator<uint16_t>>>
        synapse_to; /**< Which neuron does this synapse go to*/
    vector<vector<uint8_t, AlignmentAllocator<uint8_t>>>
        synapse_delay; /**< How much delay does this synapse have*/
    vector<vector<int8_t, AlignmentAllocator<int8_t>>>
        synapse_weight;           /**< What is this synapses weight*/
    int8_t* neuron_charge_buffer; /**< Ring buffer for each neuron, this is a
                                     2D array, with `tracked_timesteps_count`
                                     rows, and `neuron_count` cols*/
    vector<uint8_t, AlignmentAllocator<uint8_t>>
        neuron_leak; /**< Bitmap, Cannot use vector<bool> as it does not allow
                        direct access to the backing store, and is
                        implementation dependent*/

    size_t current_timestep; /**< This is what get_time() returns. */
    double min_potential; /**< At the end of a timestep, pin the charge to this
                             if less than. */
    char leak_mode; /**< 'a' for all, 'n' for nothing, 'c' for configurable */
    double spike_value_factor;
};

class Processor : public neuro::Processor {
  public:
    Processor(json& params);
    ~Processor();

    bool load_network(neuro::Network* n, int network_id = 0);
    bool load_networks(std::vector<neuro::Network*>& n);
    void clear(int network_id = 0);

    /* Queue spike(s) as input to a network or to multiple networks */

    void apply_spike(const Spike& s, bool normalized = true,
                     int network_id = 0);
    void apply_spike(const Spike& s, const vector<int>& network_ids,
                     bool normalized = true);

    void apply_spikes(const vector<Spike>& s, bool normalized = true,
                      int network_id = 0);
    void apply_spikes(const vector<Spike>& s, const vector<int>& network_ids,
                      bool normalized = true);

    /* Run the network(s) for the desired time with queued inputs */

    void run(double duration, int network_id = 0);
    void run(double duration, const vector<int>& network_ids);

    /* Get processor time based on specified network */
    double get_time(int network_id = 0);

    /* Output tracking. NOTE: Probably won't implement exactly, or at all */
    bool track_output_events(int output_id, bool track = true,
                             int network_id = 0);
    bool track_neuron_events(uint32_t node_id, bool track = true,
                             int network_id = 0);

    /* Access output spike data */
    double output_last_fire(int output_id, int network_id = 0);
    vector<double> output_last_fires(int network_id = 0);

    int output_count(int output_id, int network_id = 0);
    vector<int> output_counts(int network_id = 0);

    vector<double> output_vector(int output_id, int network_id = 0);
    vector<vector<double>> output_vectors(int network_id = 0);

    /* Spike data from all neurons. */

    long long total_neuron_counts(int network_id = 0);
    long long total_neuron_accumulates(int network_id = 0);
    vector<int> neuron_counts(int network_id = 0);
    vector<double> neuron_last_fires(int network_id = 0);
    vector<vector<double>> neuron_vectors(int network_id = 0);

    vector<double> neuron_charges(int network_id = 0);

    void synapse_weights(vector<uint32_t>& pres, vector<uint32_t>& posts,
                         vector<double>& vals, int network_id = 0);

    /* Remove state, keep network loaded */
    void clear_activity(int network_id = 0);

    /* Network/Processor Properties */
    PropertyPack get_network_properties() const;
    json get_processor_properties() const;
    json get_params() const;
    string get_name() const;

  protected:
    vrisp::Network* get_vrisp_network(int network_id);
    map<int, vrisp::Network*> networks;

    double min_weight;
    double max_weight;
    double min_threshold;
    double max_threshold;
    double min_potential;
    string leak_mode;
    double spike_value_factor;
    size_t tracked_timesteps_count;

    uint32_t min_delay;
    uint32_t max_delay;

    json saved_params;
};
} // namespace vrisp

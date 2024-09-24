#pragma once

#include <map>
#include <list>
#include "framework.hpp"
#include "nlohmann/json.hpp"
#include "utils/MOA.hpp"

using namespace neuro;
using namespace std;
namespace risp
{

class Synapse;
class Neuron;
class Network;

class Neuron {
public:
  Neuron(uint32_t node_id, double t, bool l);
  vector <Synapse*> synapses;  /**< Outgoing synapses */
  vector <double> fire_times;  /**< Firing times */ 
  double charge;               /**< Charge value */
  double threshold;            /**< Threshold value */
  int last_check;              /**< Last time we process this node */
  int last_fire;               /**< Last firing time */
  uint32_t fire_counts;        /**< Number of fires */
  bool leak;                   /**< Leak on this neuron or not */
  uint32_t id;                 /**< ID for logging events */
  bool check;                  /**< True if we have checked if this neruon fires or not */
  bool track;                  /**< True if fire_times is being tracked */
  void perform_fire(int time); /**< Perform the firing actions */
};

class Synapse {
public:
  Synapse(double w, uint32_t d, Neuron* to_n);
  double weight;             /**< Weight value */
  Neuron *to;                /**< To neuron on this synapse */
  uint32_t delay;            /**< Delay value */
};

class Network {
public:

  /** convert network in framework format to an internal risp network. */

  Network(neuro::Network *net, 
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
          vector <double> & _stds);  
  ~Network();

  /* Similar calls from Processor API */
  void apply_spike(const Spike& s, bool normalized = true);
  void run(double duration);
  double get_time();
  bool track_output_events(int output_id, bool track);
  bool track_neuron_events(uint32_t node_id, bool track);

  double output_last_fire(int output_id);
  vector <double> output_last_fires();

  int output_count(int output_id);
  vector <int> output_counts();

  vector <double> output_vector(int output_id);
  vector < vector <double> > output_vectors();

  long long total_neuron_counts();
  long long total_neuron_accumulates();
  vector <int> neuron_counts();
  vector <double> neuron_last_fires();
  vector < vector <double> > neuron_vectors();

  vector < double > neuron_charges();
  void synapse_weights(vector <uint32_t> &pres, vector <uint32_t> &posts, vector <double> &vals);

  void clear_activity();


protected:
  Neuron* add_neuron(uint32_t node_id, double threshold, bool leak);
  Synapse* add_synpase(uint32_t from_id, uint32_t to_id, double weight, uint32_t delay);

  void add_input(uint32_t node_id, int input_id);
  void add_output(uint32_t node_id, int output_id);

  Neuron* get_neuron(uint32_t node_id);
  bool is_neuron(uint32_t node_id);
  bool is_valid_output_id(int output_id);
  bool is_valid_input_id(int input_id);

  void clear_tracking_info();   /**< Clear out all tracking info to begin run() */
  
  void process_events(uint32_t time); /**< Process events at time "time" */

  vector <int> inputs;        /**< index is input id and its value is neuron id. 
                                   If the neuron id is -1, it's not an input node. */
  vector <int> outputs;       /**< index is output id and its value is neuron id. 
                                   If the neuron id is -1, it's not an ouput node. */
  vector <Neuron *> sorted_neuron_vector;         /**< sorted neurons by node id */

  unordered_map <uint32_t, Neuron*> neuron_map;   /**< key is neuron id */

  /** The index of the vector is the timestep.
   *  Each subvector stores a set of events, which is composed of neuron and charge change.
   */
  vector < vector < std::pair<Neuron *, double> >> events;

  long long neuron_fire_counter;  /**< This is what total_neuron_counts() returns. */
  long long neuron_accum_counter; /**< This is what total_neuron_accumulates() returns. */
  int overall_run_time;     /**< This is what get_time() returns. */
  bool run_time_inclusive;  /**< Do we run for duration+1 (true) or duration (false) */
  bool threshold_inclusive; /**< Do we spike on >= or >. */
  double min_potential;     /**< At the end of a timestep, pin the charge to this if less than. */
  bool fire_like_ravens;    /**< Register neuron fires one step later, so RISP can match RAVENS. */
  bool discrete;            /**< Are networks discrete */
  bool inputs_from_weights; /**< Inputs are indices into the weight vector. */
  char leak_mode;           /**< 'a' for all, 'n' for nothing, 'c' for configurable */
  vector <double> weights;
  double noisy_stddev;
  uint32_t noisy_seed;
  MOA rng;
  vector <double> stds;

  double spike_value_factor;
  vector <Neuron *> to_fire;   /* To make RISP like RAVENS, this lets you fire a timestep later. */

};

class Processor : public neuro::Processor
{
public:
  Processor(json &params);
  ~Processor();

  bool load_network(neuro::Network* n, int network_id = 0);
  bool load_networks(std::vector<neuro::Network*> &n);
  void clear(int network_id = 0);
  
  /* Queue spike(s) as input to a network or to multiple networks */

  void apply_spike(const Spike& s, bool normalized = true, int network_id = 0);
  void apply_spike(const Spike& s, const vector<int>& network_ids, bool normalized = true);

  void apply_spikes(const vector<Spike>& s, bool normalized = true, int network_id = 0);
  void apply_spikes(const vector<Spike>& s, const vector<int>& network_ids, bool normalized = true);

  /* Run the network(s) for the desired time with queued input(s) */

  void run(double duration, int network_id = 0);
  void run(double duration, const vector<int>& network_ids);

  /* Get processor time based on specified network */
  double get_time(int network_id = 0);

  /* Output tracking.  See the markdown for a detailed description of these.  */

  bool track_output_events(int output_id, bool track = true, int network_id = 0);
  bool track_neuron_events(uint32_t node_id, bool track = true, int network_id = 0);

  /* Access output spike data */

  double output_last_fire(int output_id, int network_id = 0);
  vector <double> output_last_fires(int network_id = 0);

  int output_count(int output_id, int network_id = 0);
  vector <int> output_counts(int network_id = 0);

  vector <double> output_vector(int output_id, int network_id = 0);
  vector < vector <double> > output_vectors(int network_id = 0);

  /* Spike data from all neurons. */

  long long total_neuron_counts(int network_id = 0);
  long long total_neuron_accumulates(int network_id = 0);
  vector <int> neuron_counts(int network_id = 0);
  vector <double> neuron_last_fires(int network_id = 0);
  vector < vector <double> > neuron_vectors(int network_id = 0);

  vector < double > neuron_charges(int network_id = 0);

  void synapse_weights(vector <uint32_t> &pres,
                       vector <uint32_t> &posts,
                       vector <double> &vals,
                       int network_id = 0);

  /* Remove state, keep network loaded */
  void clear_activity(int network_id = 0);

  /* Network and Processor Properties.  The network properties correspond to the Data
     field in the network, nodes and edges.  The processor properties are so that
     applications may query the processor for various properties (e.g. input scaling,
     fire-on-threshold vs fire-over-threshold. */

  PropertyPack get_network_properties() const;
  json get_processor_properties() const;
  json get_params() const;
  string get_name() const;

protected:

  risp::Network* get_risp_network(int network_id);
  double get_input_spike_factor() const;
  map <int, risp::Network*> networks;

  double min_weight;
  double max_weight;
  double min_threshold;
  double max_threshold;
  double min_potential;
  double noisy_stddev;
  bool discrete;
  double spike_value_factor;
  string leak_mode;
  bool run_time_inclusive;
  bool threshold_inclusive;
  bool fire_like_ravens;
  bool inputs_from_weights;
  uint32_t noisy_seed;
  vector <double> weights;
  vector <double> stds;

  uint32_t min_delay;
  uint32_t max_delay;

  json saved_params;

};

}

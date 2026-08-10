#include <deque>
#include <vector>
#include <unordered_map>
#include "framework.hpp"
#include "nlohmann/json.hpp"

namespace izhikevich {

  class Synapse;
  class Network;

  class Neuron {
  public:

    double v;
    double u;

    double a;
    double b;
    double c;
    double d;

    double input;

    int id;

    bool tracking;
    int last_fire_time;
    int fire_count;
    std::vector<double> fire_times;

    neuro::Node* node;

    std::vector<Synapse *> synapses;

    Neuron(neuro::Node* node, int id);
    ~Neuron();

    void update();
    void fire(Network* net, int time);
    void reset();
  };

  class Synapse {
  public:
    Neuron* to;
    int weight;
    int delay;

    Synapse(Neuron* to, neuro::Edge* edge, bool exc);
  };

  class Network {
  public:
    std::vector<int> inputs;
    std::vector<int> outputs;
    std::vector<Neuron *> neurons;
    std::unordered_map<uint32_t, int> ids;

    int timestep;
    long long total_fire_count;
    long long total_accumulate_count;

    /* Events have the form <neuron id, value>. */

    std::deque< std::vector< std::pair<int, double> > > events;

    Network(neuro::Network* network);
    ~Network();

    void run(int duration);
    Neuron* get_neuron(uint32_t node_id);
    Neuron* get_output(int output_id);
    void reset();
  };

  class Processor : public neuro::Processor {
  public:
    static nlohmann::json spec;
    neuro::PropertyPack ppack;
    nlohmann::json properties;
    nlohmann::json params;
    int input_scaling_value;

    std::unordered_map<int, Network *> networks;

    Processor(nlohmann::json& arg);
    ~Processor();

    bool load_network(neuro::Network* network, int network_id = 0);
    bool load_networks(std::vector<neuro::Network *> &networks);

    void clear(int network_id = 0);

    void apply_spike(
      const neuro::Spike& spike,
      bool normalized = true,
      int network_id = 0
    );

    void apply_spike(
      const neuro::Spike &spike,
      const std::vector<int>& network_ids,
      bool normalized = true
    );

    void apply_spikes(
      const std::vector<neuro::Spike>& spikes,
      bool normalized = true,
      int network_id = 0
    );

    void apply_spikes(
      const std::vector<neuro::Spike>& spikes,
      const std::vector<int>& network_ids,
      bool normalized = true
    );

    void run(double duration, int network_id = 0);
    void run(double duration, const std::vector<int>& network_ids);

    double get_time(int network_id = 0);

    bool track_output_events(
      int output_id,
      bool track = true,
      int network_id = 0
    );

    bool track_neuron_events(
      uint32_t node_id,
      bool track = true,
      int network_id = 0
    );

    double output_last_fire(int output_id, int network_id = 0);
    std::vector<double> output_last_fires(int network_id = 0);
    int output_count(int output_id, int network_id = 0);
    std::vector<int> output_counts(int network_id = 0);

    std::vector<double> output_vector(
      int output_id,
      int network_id = 0
    );

    std::vector< std::vector<double> > output_vectors(
      int network_id = 0
    );

    long long total_neuron_counts(int network_id = 0);
    long long total_neuron_accumulates(int network_id = 0);
    std::vector<int> neuron_counts(int network_id = 0);
    std::vector<double> neuron_last_fires(int network_id = 0);

    std::vector< std::vector<double> > neuron_vectors(
      int network_id = 0
    );

    std::vector<double> neuron_charges(int network_id = 0);

    void synapse_weights(
      std::vector<uint32_t>& pres,
      std::vector<uint32_t>& posts,
      std::vector<double>& vals,
      int network_id = 0
    );

    void clear_activity(int network_id = 0);

    neuro::PropertyPack get_network_properties() const;
    nlohmann::json get_processor_properties() const;
    nlohmann::json get_params() const;

    std::string get_name() const;

    Network* get_network(int network_id);
  };
}

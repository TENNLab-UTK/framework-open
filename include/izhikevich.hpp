#include <vector>
#include <unordered_map>
#include "framework.hpp"
#include "nlohmann/json.hpp"

namespace izhikevich {
    class Synapse;
    class Network;
    class Processor;

    class Neuron {
    public:
        Neuron(
          neuro::Node* node,
          const Processor* p,
          int index);
        ~Neuron();
        void update();
        bool has_to_fire() const;
        void fire();
        void set_network(Network* network);
        void reset_input_current();
        void reset_firing_info();
        void reset_state();

        double v;
        double u;
        double a;
        double b;
        double c;
        double d;
        double I;

        const double& tau;

        int index;
        int node_id;

        bool tracking;
        bool fired;
        int last_fire;
        int fire_count;
        std::vector<double> fire_times;

        std::vector<izhikevich::Synapse*> synapses;

        neuro::Node* node;
        Network* net;
    };

    class Synapse {
    public:
        Neuron* from;
        Neuron* to;
        double current;
        int delay;

        Synapse(Neuron* from, Neuron* to, neuro::Edge* edge);
    };

    class Network {
    public:
        std::unordered_map<uint32_t, int> indices;
        std::vector<Neuron*> neurons;
        std::vector<int> inputs;
        std::vector<int> outputs;
        int timestep;
        int nsynapses;
        long long total_fires;
        long long total_accumulates;

        std::unordered_map<int,
          std::unordered_map<int, double>> events;

        Network(neuro::Network* network, const Processor* p);
        ~Network();
        Neuron* get_neuron(uint32_t node_id) const;
        Neuron* get_output(int output_id) const;
    };

    class Processor : public neuro::Processor {
    public:
        static nlohmann::json spec;
        neuro::PropertyPack ppack;
        nlohmann::json properties;
        nlohmann::json params;
        double min_input_current;
        double max_input_current;
        double milliseconds_per_timestep;
        double tau;

        std::unordered_map<int, Network*> networks;

        Network* get_network(int network_id) const;

        Processor(nlohmann::json& arg);
        ~Processor();
        bool load_network(neuro::Network* network, int network_id = 0);
        bool load_networks(std::vector<neuro::Network*>& networks);
        void clear(int network_id = 0);
        void apply_spike(
          const neuro::Spike& spike,
          bool normalize = true,
          int network_id = 0);
        void apply_spike(
          const neuro::Spike& spike,
          const std::vector<int>& network_ids,
          bool normalize = true);
        void apply_spikes(
          const std::vector<neuro::Spike>& spikes,
          bool normalize = true,
          int network_id = 0);
        void apply_spikes(
          const std::vector<neuro::Spike>& spikes,
          const std::vector<int>& network_ids,
          bool normalize = true);
        void run(double duration, int network_id = 0);
        void run(double duration, const std::vector<int>& network_ids);
        double get_time(int network_id = 0);
        bool track_output_events(
          int output_id,
          bool track = true,
          int network_id = 0);
        bool track_neuron_events(
          uint32_t node_id,
          bool track = true,
          int network_id = 0);
        double output_last_fire(int output_id, int network_id = 0);
        std::vector<double> output_last_fires(int network_id = 0);
        int output_count(int output_id, int network_id = 0);
        std::vector<int> output_counts(int network_id = 0);
        std::vector<double> output_vector(
          int output_id,
          int network_id = 0);
        std::vector<std::vector<double>> output_vectors(
          int network_id = 0);
        long long total_neuron_counts(int network_id = 0);
        long long total_neuron_accumulates(int network_id = 0);
        std::vector<int> neuron_counts(int network_id = 0);
        std::vector<double> neuron_last_fires(int network_id = 0);
        std::vector<std::vector<double>> neuron_vectors(
          int network_id = 0);
        std::vector<double> neuron_charges(int network_id = 0);
        void synapse_weights(
          std::vector<uint32_t>& pres,
          std::vector<uint32_t>& posts,
          std::vector<double>& vals,
          int network_id = 0);
        void clear_activity(int network_id = 0);
        neuro::PropertyPack get_network_properties() const;
        nlohmann::json get_processor_properties() const;
        nlohmann::json get_params() const;
        std::string get_name() const;
    };
}

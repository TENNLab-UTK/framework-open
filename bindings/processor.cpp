#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include "pybind_json.hpp"

#include "framework.hpp"
#include "nlohmann/json.hpp"

namespace neuro
{
    class PyProcessor : public Processor
    {
    public:
        using Processor::Processor;

        bool load_network(Network* n, int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(
                bool,           /* return value */
                Processor,      /* parent class */
                load_network,   /* method name  */
                n,              /* arguments... */
                network_id
            );
        }

        bool load_networks(vector<Network*>& n) override
        {
            PYBIND11_OVERLOAD_PURE(bool, Processor, load_networks, n);
        }

        void apply_spike(const Spike& s, bool normalized = true, int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(void, Processor, apply_spike, s, normalized, network_id);
        }

        void apply_spikes(const vector<Spike>& s, bool normalized = true, int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(void, Processor, apply_spikes, s, normalized, network_id);
        }

        void apply_spike(const Spike& s, const vector<int>& network_ids, bool normalized = true) override
        {
            PYBIND11_OVERLOAD_PURE(void, Processor, apply_spike, s, network_ids, normalized);
        }

        void apply_spikes(const vector<Spike>& s, const vector<int>& network_ids, bool normalized = true) override
        {
            PYBIND11_OVERLOAD_PURE(void, Processor, apply_spikes, s, network_ids, normalized);
        }

        void run(double duration, int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(void, Processor, run, duration, network_id);
        }

        void run(double duration, const vector<int>& network_ids) override
        {
            PYBIND11_OVERLOAD_PURE(void, Processor, run, duration, network_ids);
        }

        double get_time(int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(double, Processor, get_time, network_id);
        }

        bool track_output_events(int output_id, bool track = true, int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(bool, Processor, track_output_events, output_id, track, network_id);
        }

        bool track_neuron_events(uint32_t node_id, bool track = true, int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(bool, Processor, track_neuron_events, node_id, track, network_id);
        }

        double output_last_fire(int output_id, int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(double, Processor, output_last_fire, output_id, network_id);
        }

        vector<double> output_last_fires(int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(vector<double>, Processor, output_last_fires, network_id);
        }

        int output_count(int output_id, int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(int, Processor, output_count, output_id, network_id);
        }

        vector<int> output_counts(int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(vector<int>, Processor, output_counts, network_id);
        }

        vector<double> output_vector(int output_id, int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(vector<double>, Processor, output_vector, output_id, network_id);
        }

        vector< vector<double>> output_vectors(int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(vector< vector<double>>, Processor, output_vectors, network_id);
        }

        long long total_neuron_counts(int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(long long, Processor, total_neuron_counts, network_id);
        }

        long long total_neuron_accumulates(int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(long long, Processor, total_neuron_accumulates, network_id);
        }

        vector<int> neuron_counts(int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(vector<int>, Processor, neuron_counts, network_id);
        }

        vector<double> neuron_last_fires(int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(vector<double>, Processor, neuron_last_fires, network_id);
        }

        vector< vector<double>> neuron_vectors(int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(vector< vector<double>>, Processor, neuron_vectors, network_id);
        }

        void clear(int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(void, Processor, clear, network_id);
        }

        void clear_activity(int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(void, Processor, clear_activity, network_id);
        }

        PropertyPack get_network_properties() const override
        {
            PYBIND11_OVERLOAD_PURE(PropertyPack, Processor, get_network_properties);
        }

        nlohmann::json get_processor_properties() const override
        {
            PYBIND11_OVERLOAD_PURE(nlohmann::json, Processor, get_processor_properties);
        }
        nlohmann::json get_params() const override
        {
            PYBIND11_OVERLOAD_PURE(nlohmann::json, Processor, get_params);
        }
        string get_name() const override
        {
            PYBIND11_OVERLOAD_PURE(string, Processor, get_name);
        }
        vector <double> neuron_charges(int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(vector <double>, Processor, neuron_charges, network_id);
        }
        void synapse_weights(vector <uint32_t> &pres,
                                  vector <uint32_t> &posts,
                                  vector <double> &vals,int network_id = 0) override
        {
            PYBIND11_OVERLOAD_PURE(void, Processor, synapse_weights, pres, posts, vals, network_id);
        }
    };
}


void bind_framework_processor(pybind11::module &m) {
    namespace py = pybind11;
    using std::vector;
    using std::string;

    // TODO: Add Batch operations

    /* Spikes are essentially 3-tuples of id, time, and value */
    py::class_<neuro::Spike>(m, "Spike")
        .def(py::init<int,double,double>(), py::arg("id"), py::arg("time"), py::arg("value"))
        .def_readwrite("id", &neuro::Spike::id)
        .def_readwrite("time", &neuro::Spike::time)
        .def_readwrite("value", &neuro::Spike::value);

    /* Requires a trampoline class for override/inheritance of virutal methods */
    py::class_<neuro::Processor, neuro::PyProcessor>(m, "Processor")
        .def(py::init<>())
        .def("get_params",      &neuro::Processor::get_processor_properties)
        .def("get_name",      &neuro::Processor::get_network_properties)

        .def("get_processor_properties",      &neuro::Processor::get_processor_properties)
        .def("get_network_properties",      &neuro::Processor::get_network_properties)
        .def("get_time",            &neuro::Processor::get_time,
                py::arg("network_id") = 0)

        .def("load_network",        &neuro::Processor::load_network,
                py::arg("network"), py::arg("network_id") = 0)

        .def("load_networks",       &neuro::Processor::load_networks,
                py::arg("network"))

        .def("apply_spike",         (void (neuro::Processor::*)(const neuro::Spike&, bool, int)) &neuro::Processor::apply_spike,
                py::arg("spike"), py::arg("normalized") = true, py::arg("network_id") = 0)

        .def("apply_spikes",        (void (neuro::Processor::*)(const vector<neuro::Spike>&, bool, int)) &neuro::Processor::apply_spikes,
                py::arg("spikes"), py::arg("normalized") = true, py::arg("network_id") = 0)

        .def("run",                 (void (neuro::Processor::*)(double, int)) &neuro::Processor::run,
                py::arg("duration"), py::arg("network_id") = 0)

        .def("get_time",            &neuro::Processor::get_time,
                py::arg("network_id") = 0)

        .def("track_output_events", &neuro::Processor::track_output_events,
                py::arg("output_id"), py::arg("track") = true, py::arg("network_id") = 0)

        .def("track_neuron_events", &neuro::Processor::track_neuron_events,
                py::arg("node_id"), py::arg("track") = true, py::arg("network_id") = 0)

        .def("output_last_fires",   (vector<double> (neuro::Processor::*)(int)) &neuro::Processor::output_last_fires,
                py::arg("network_id") = 0)

        .def("output_last_fire",   (double (neuro::Processor::*)(int, int)) &neuro::Processor::output_last_fire,
                py::arg("id"), py::arg("network_id") = 0)

        .def("clear",               &neuro::Processor::clear,
                py::arg("network_id") = 0)

        .def("clear_activity",      &neuro::Processor::clear_activity,
                py::arg("network_id") = 0)

        .def("output_count",        &neuro::Processor::output_count,
                py::arg("output_id"), py::arg("network_id") = 0)

        .def("output_counts",        &neuro::Processor::output_counts,
                py::arg("network_id") = 0)

        .def("output_vector",       &neuro::Processor::output_vector,
                py::arg("output_id"), py::arg("network_id") = 0)

        .def("output_vectors",       &neuro::Processor::output_vectors,
                py::arg("network_id") = 0)

        .def("total_neuron_counts",  &neuro::Processor::total_neuron_counts,
                py::arg("network_id") = 0)

        .def("total_neuron_accumulates", &neuro::Processor::total_neuron_accumulates,
                py::arg("network_id") = 0)

        .def("neuron_counts",      &neuro::Processor::neuron_counts,
                py::arg("network_id") = 0)

        .def("neuron_vectors",      &neuro::Processor::neuron_vectors,
                py::arg("network_id") = 0)

        .def("neuron_last_fires",      &neuro::Processor::neuron_last_fires,
                py::arg("network_id") = 0)

        .def("neuron_charges",      &neuro::Processor::neuron_charges,
                py::arg("network_id") = 0)
        .def("synapse_weights", [](neuro::Processor &proc, int network_id = 0) {
            vector <uint32_t> pres;
            vector <uint32_t> posts;
            vector <double> vals;
            proc.synapse_weights(pres, posts, vals, network_id);
            return make_tuple(pres, posts, vals);
        },  py::arg("network_id")=0)
        /* Below are extra methods provided for ease of use and performance reasons. */

        /* Apply binary data as a spikes for each bit place.
         * TODO: implement version with two neurons per bit place */
        .def("apply_binary_data", [](neuro::Processor &dev, const std::vector<uint8_t>& ram_data, bool two_neurons) {
            vector<neuro::Spike> spikes;

            for(auto i = 0; i < ram_data.size(); i++)
            {
                for(auto b = 0; b < 8; b++)
                {
                    if(ram_data[i] & (1 << b))
                    {
                        spikes.emplace_back(8*i+b, 0, 1);
                    }
                }
            }

            dev.apply_spikes(spikes);

         }, py::arg("ram_data"), py::arg("two_neurons") = false)

        /* Apply DVS events from set of vectors (x, y, polarity, time) given x,y dimensions. */
        .def("apply_dvs_events",
        [](neuro::Processor &dev,
           const std::vector<int> x,
           const std::vector<int> y,
           const std::vector<int> p,
           const std::vector<double> t,
           std::pair<int,int> dims,
           bool use_polarity) {

            // error check
            if(x.size() != y.size() || y.size() != t.size() || (use_polarity && t.size() != p.size())) {
                throw std::runtime_error("[apply_dvs_events] x, y, p, and t must have matching length");
            }

            // Determine coordinate bounds
            uint32_t max_x, max_y;
            std::tie(max_x, max_y) = dims;
            uint32_t frame_size = max_x * max_y;

            // Generate and immediately apply spikes
            for(size_t i = 0; i < x.size(); ++i)
            {
                auto nid = y[i] * max_x + x[i];
                if(use_polarity) nid += p[i] * frame_size;
                neuro::Spike s(nid, floor(t[i]), 1.0);
                dev.apply_spike(s);
            }

        }, py::arg("x"), py::arg("y"), py::arg("p"), py::arg("t"), py::arg("dims"), py::arg("use_polarity") = true)

        /* Get (index, value) of the output neuron which produced the most ouptut spikes. */
        .def("output_count_max", [](neuro::Processor &dev, int num_outputs, int network_id) {
            int idx = 0;
            int val = -1;
            int tmp = 0;

            for(int oid = 0; oid < num_outputs; ++oid)
            {
                tmp = dev.output_count(oid, network_id);

                if(tmp > val)
                {
                    idx = oid;
                    val = tmp;
                }
            }

            return std::make_tuple(idx, val);
        }, py::arg("num_outputs"), py::arg("network_id") = 0)

        /* Get a list of all output spike counts. */
        .def("output_count_all", [](neuro::Processor &dev, int network_id, int num_outputs) {
            std::vector<int> counts(num_outputs);

            for(int oid = 0; oid < num_outputs; ++oid)
            {
                counts[oid] = dev.output_count(oid, network_id);
            }

            return counts;
          }, py::arg("network_id"), py::arg("num_outputs"));
}

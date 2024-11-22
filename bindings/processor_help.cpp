#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "framework.hpp"

namespace py = pybind11;

void bind_processor_help(py::module &m) {

    using namespace neuro;
    
    m.def("track_all_output_events", &neuro::track_all_output_events,
          py::arg("p"), py::arg("n"),
          py::arg("network_id") = 0);

    m.def("track_all_neuron_events", &neuro::track_all_neuron_events,
          py::arg("p"), py::arg("n"),
          py::arg("network_id") = 0);

    m.def("neuron_counts_to_json", &neuro::neuron_counts_to_json, 
        py::arg("counts"), py::arg("n"));

    m.def("neuron_last_fires_to_json", &neuro::neuron_last_fires_to_json, 
        py::arg("last_fires"), py::arg("n"));

    m.def("neuron_vectors_to_json", &neuro::neuron_vectors_to_json,
        py::arg("events"), py::arg("type"), py::arg("n"));

	m.def("apply_periodic_spikes", py::overload_cast<neuro::Processor*, const neuro::Period&, int>(&neuro::apply_periodic_spikes),
			py::arg("p"), py::arg("period"), py::arg("network_id") = 0);

	m.def("apply_periodic_spikes", py::overload_cast<neuro::Processor*, const neuro::Period&, const vector<int>&>(&neuro::apply_periodic_spikes),
			py::arg("p"), py::arg("period"), py::arg("network_ids"));

}

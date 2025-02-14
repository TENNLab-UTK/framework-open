#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <memory>

#include "framework.hpp"
#include "nlohmann/json.hpp"

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
    
    m.def("neuron_charges_to_json", &neuro::neuron_charges_to_json,
        py::arg("charges"), py::arg("n"));

	m.def("run_and_track", &neuro::run_and_track,
		py::arg("duration"), py::arg("p"), py::arg("network_id") = 0);

	m.def("apply_spike_raster", &neuro::apply_spike_raster,
		py::arg("p"), py::arg("in_neuron"), py::arg("sr"), py::arg("network_id") = 0);

	m.def("pull_network", &neuro::pull_network,
		py::arg("p"), py::arg("n"), py::arg("network_id") = 0);
}

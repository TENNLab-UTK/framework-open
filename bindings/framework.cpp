#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl_bind.h>

#include "framework.hpp"
#include "nlohmann/json.hpp"
#include "pybind_json.hpp"

namespace py = pybind11;
using namespace neuro;

void bind_framework_network(py::module &m);
void bind_framework_processor(py::module &m);
void bind_processor_help(py::module &m);

PYBIND11_MODULE(neuro, m) {
    m.doc() = "Neuromorphic Framework";

	// Bind Nodes, Edges, & Networks 
	bind_framework_network(m);

	// Bind Device Interface & Spikes
	bind_framework_processor(m);

	//Bind helper functions for processors
	bind_processor_help(m);
}

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

	py::class_<nlohmann::json>(m, "json", py::dynamic_attr())
        .def(py::init<>())
        .def(py::init<py::object>())
        .def("to_python", [](nlohmann::json &j){
            py::object obj = j;
            return obj;
        })
        .def(py::pickle(
            [](nlohmann::json &j){
                py::object obj = json_to_py(j);
                return obj;
            },
            [](py::object o){
                nlohmann::json j = py_to_json(o);
                return j;
            }
        ))
        .def("__repr__", [](nlohmann::json &j) {
            return j.dump();
        })
        .def("__str__", [](nlohmann::json &j) {
            return j.dump(4);
        });
    py::implicitly_convertible<py::object, nlohmann::json>();

	// Bind Nodes, Edges, & Networks 
	bind_framework_network(m);

	// Bind Device Interface & Spikes
	bind_framework_processor(m);

	//Bind helper functions for processors
	bind_processor_help(m);
}

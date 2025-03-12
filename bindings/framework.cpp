#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl_bind.h>

#include "framework.hpp"
#include "nlohmann/json.hpp"
#include "pybind_json.hpp"
#include "utils/json_helpers.hpp"

namespace py = pybind11;
using namespace neuro;

void bind_framework_network(py::module &m);
void bind_framework_processor(py::module &m);
void bind_processor_help(py::module &m);
void bind_framework_moa(py::module &m);

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
		.def("__getitem__", [](nlohmann::json &j, const py::object &key) {
			// Checks the type of the key, then returns the value based on its type if it exists
			if (py::isinstance<py::str>(key)) {
				auto value = j[py::cast<std::string>(key)];
			
				if (value.is_string()) {
					return py::cast(value.get<std::string>());
				} else if (value.is_number_integer()) {
					return py::cast(value.get<int>());
				} else if (value.is_number_float()) {
					return py::cast(value.get<float>());
				} else if (value.is_boolean()) {
					return py::cast(value.get<bool>());
				} else if (value.is_null()) {
					return py::cast<py::object>(py::none());
				} else if (value.is_object() || value.is_array()) {
					return py::cast(value);
				} else {
					throw py::type_error("Unknown return value type");
				}

			} else if (py::isinstance<py::int_>(key)) {
				auto value = j[py::cast<std::size_t>(key)];

				if (value.is_string()) {
					return py::cast(value.get<std::string>());
				} else if (value.is_number_integer()) {
					return py::cast(value.get<int>());
				} else if (value.is_number_float()) {
					return py::cast(value.get<float>());
				} else if (value.is_boolean()) {
					return py::cast(value.get<bool>());
				} else if (value.is_null()) {
					return py::cast<py::object>(py::none());
				} else if (value.is_object() || value.is_array()) {
					return py::cast(value);
				} else {
					throw py::type_error("Unknown return value type");
				}

			} else {
				throw py::type_error("Key must be string or int");
			}
		})
		.def("__setitem__", [](nlohmann::json &j, const py::object &key, const py::object &o) {
			// Checks the type of the key and sets the value
			if (py::isinstance<py::str>(key)) {
				j[py::cast<std::string>(key)] = o;
			} else if (py::isinstance<py::int_>(key)) {
				j[py::cast<std::size_t>(key)] = o;
			} else {
				throw py::type_error("Key must be string or int");
			}
		})
		.def("__contains__", [](nlohmann::json &j, const py::object &key) {
			// Checks the type of the key and returns whether or not the key exists
			if (py::isinstance<py::str>(key)) {
				if (j.is_object()) {
					return j.contains(py::cast<std::string>(key));
				}
			} else if (py::isinstance<py::int_>(key)) {
				if (j.is_array()) {
					size_t index = py::cast<std::size_t>(key);
					return index < j.size();
				}
			}
			return false;
		})
        .def("__repr__", [](nlohmann::json &j) {
            return j.dump();
        })
        .def("__str__", [](nlohmann::json &j) {
            return j.dump();
        })
		.def("__len__", [](nlohmann::json &j) {
			return j.size();
			})
		.def("dump", [](nlohmann::json &j, int val) {
			return j.dump(val);
		})
		.def(py::self == py::self);
	py::implicitly_convertible<py::object, nlohmann::json>();

	m.def("pretty_json_helper", &pretty_json_helper,
		 py::arg("j"),
		 py::arg("indent_space") = 2,
		 py::arg("indent") = 2,
		 py::arg("print_width") = 150,
		 py::arg("indent_bracket") = true);
	
    py::implicitly_convertible<py::object, nlohmann::json>();

	// Bind Nodes, Edges, & Networks 
	bind_framework_network(m);

	// Bind Device Interface & Spikes
	bind_framework_processor(m);

	// Bind helper functions for processors
	bind_processor_help(m);

	// Bind MOA RNG class
	bind_framework_moa(m);
}

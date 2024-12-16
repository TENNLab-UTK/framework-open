#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "pybind_json.hpp"
#include "framework.hpp"
#include "nlohmann/json.hpp"

// change includes to match your code
#include "risp.hpp" 

namespace py = pybind11;

 // "superprocessor" should be the name of the python module
PYBIND11_MODULE(risp, m) {
  m.doc() = "risp";
  py::module::import("neuro");
  py::object n_processor = (py::object) py::module::import("neuro").attr("Processor");

  // replace "super::" with the correct namespace for your implementation
  py::class_<risp::Processor>(m, "Processor", n_processor)
    .def(py::init<nlohmann::json&>());
}

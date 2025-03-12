#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "pybind_json.hpp"
#include "framework.hpp"
#include "nlohmann/json.hpp"

#include "risp.hpp" 

namespace py = pybind11;

PYBIND11_MODULE(risp, m) {
  m.doc() = "risp";
  py::module::import("neuro");

  py::class_<risp::Processor, neuro::Processor>(m, "Processor", py::multiple_inheritance())
    .def(py::init<nlohmann::json&>());
}

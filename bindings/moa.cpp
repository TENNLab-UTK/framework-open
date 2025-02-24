#include <pybind11/pybind11.h>

#include "utils/MOA.hpp"

namespace py = pybind11;

void bind_framework_moa(py::module &m) {
    py::class_<neuro::MOA>(m, "MOA")
        .def(py::init<>())
        .def("Random_Double",       &neuro::MOA::Random_Double)
        .def("Random_DoubleI",      &neuro::MOA::Random_DoubleI)
        .def("Random_Integer",      &neuro::MOA::Random_Integer)
        .def("Random_32",           &neuro::MOA::Random_32)
        .def("Random_64",           &neuro::MOA::Random_64)
        .def("Random_128",          &neuro::MOA::Random_128)
        .def("Random_w",            &neuro::MOA::Random_W)
        .def("Seed",                &neuro::MOA::Seed)
        .def("Hash",                &neuro::MOA::Hash)
        .def("Get_State",           &neuro::MOA::Get_State)
        .def("Get_Counter",         &neuro::MOA::Get_Counter)
        .def("Set_State",           &neuro::MOA::Set_State)
		.def("Seed_From_Time",      &neuro::MOA::Seed_From_Time);
}

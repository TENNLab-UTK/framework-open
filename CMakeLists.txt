project(neuro)

find_package(pybind11 REQUIRED)

set (SOURCES bindings/framework.cpp)

pybind11_add_module(neuro ${SOURCES})

target_include_directories(neuro PRIVATE ./include)

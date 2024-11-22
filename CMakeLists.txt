project(neuro)

find_package(pybind11 REQUIRED)

set(SOURCES_NEURO bindings/framework.cpp)

pybind11_add_module(neuro ${SOURCES_NEURO})

target_include_directories(neuro PRIVATE ./include ./bindings)

set(SOURCES_RISP bindings/risp.cpp)

pybind11_add_module(risp ${SOURCES_RISP})

target_include_directories(risp PRIVATE ./include ./bindings)

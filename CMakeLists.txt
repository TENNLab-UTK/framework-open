cmake_minimum_required(VERSION 3.22)
project(neuro)

# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bindings)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../pyframework/lib/python3.10/site-packages)
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};pyframework/lib/python3.10/site-packages/pybind11/share/cmake/pybind11/")
find_package(pybind11 REQUIRED)

set(SOURCES_NEURO bindings/framework.cpp
                  bindings/processor_help.cpp
                  bindings/processor.cpp
                  bindings/network.cpp
                  bindings/pybind_json.hpp
                  src/framework.cpp
                  src/properties.cpp
                  src/processor_help.cpp)

pybind11_add_module(neuro ${SOURCES_NEURO})

target_include_directories(neuro PRIVATE ./include ./bindings)

set(SOURCES_RISP bindings/risp.cpp
                 bindings/pybind_json.hpp
                 src/risp.cpp
                 src/risp_static.cpp
                 src/framework.cpp
                 src/properties.cpp
                 src/processor_help.cpp)

pybind11_add_module(risp ${SOURCES_RISP})

target_include_directories(risp PRIVATE ./include ./bindings)

target_include_directories(neuro PRIVATE ./include)

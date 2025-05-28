cmake_minimum_required(VERSION 3.22)
project(neuro)

find_package(pybind11 REQUIRED)
find_package(Python3)

execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')"
    OUTPUT_VARIABLE PYTHON_VERSION
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Detected Python version: ${PYTHON_VERSION}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../pyframework/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages)
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};pyframework/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages/pybind11/share/cmake/pybind11/")

set(SOURCES_NEURO bindings/framework.cpp
                  bindings/processor_help.cpp
                  bindings/processor.cpp
                  bindings/network.cpp
                  bindings/pybind_json.hpp
				  bindings/moa.cpp
                  src/framework.cpp
                  src/properties.cpp
                  src/processor_help.cpp
			  	  include/utils/MOA.hpp
			  	  include/utils/json_helpers.hpp)

pybind11_add_module(neuro ${SOURCES_NEURO})

target_include_directories(neuro PRIVATE ./include ./bindings)

set(SOURCES_RISP bindings/risp.cpp
                 bindings/pybind_json.hpp
                 src/risp.cpp
                 src/risp_static.cpp
                 src/framework.cpp
                 src/properties.cpp
                 src/processor_help.cpp
			  	 include/utils/MOA.hpp
			 	 include/utils/json_helpers.hpp)

pybind11_add_module(risp ${SOURCES_RISP})

target_include_directories(risp PRIVATE ./include ./bindings)

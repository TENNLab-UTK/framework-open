#include "framework.hpp"
#include "vrisp.hpp"

neuro::Processor* neuro::Processor::make(const string& name, json& params) {
    string error_string;

    if (name != "vrisp") {
        error_string = (string) "Processor::make() called with a name (" +
                       name + (string) ") not equal to vrisp";
        throw std::runtime_error(error_string);
    }

    return new vrisp::Processor(params);
}

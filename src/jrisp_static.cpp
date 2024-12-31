#include "framework.hpp"
#include "jrisp.hpp"

neuro::Processor* neuro::Processor::make(const string& name, json& params) {
    string error_string;

    if (name != "jrisp") {
        error_string = (string) "Processor::make() called with a name (" +
                       name + (string) ") not equal to jrisp";
        throw std::runtime_error(error_string);
    }

    return new jrisp::Processor(params);
}

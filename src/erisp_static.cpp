#include "framework.hpp"
#include "erisp.hpp"

neuro::Processor *neuro::Processor::make(const string &name, json &params)
{
  string es;

  if (name != "erisp") {
    es = (string) "Processor::make() called with a name ("
       + name
       + (string) ") not equal to erisp";
    throw std::runtime_error(es);
  }

  return new erisp::Processor(params);
}

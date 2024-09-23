#include "framework.hpp"
#include "risp.hpp"

neuro::Processor *neuro::Processor::make(const string &name, json &params)
{
  string es;

  if (name != "risp") {
    es = (string) "Processor::make() called with a name ("
       + name
       + (string) ") not equal to risp";
    throw std::runtime_error(es);
  }

  return new risp::Processor(params);
}

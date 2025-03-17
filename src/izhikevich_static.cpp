#include <string>
#include "izhikevich.hpp"
#include "framework.hpp"
using nlohmann::json;

neuro::Processor *neuro::Processor::make(const string &name, json &params)
{
  string es;

  if (name != "izhikevich") {
    es = (string) "Processor::make() called with a name ("
       + name
       + (string) ") not equal to izhikevich";
    throw std::runtime_error(es);
  }

  return new izhikevich::Processor(params);
}

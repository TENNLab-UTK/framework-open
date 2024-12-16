#include <fstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/operators.h>
#include "pybind_json.hpp"

#include "framework.hpp"
#include "nlohmann/json.hpp"

namespace py = pybind11;

void bind_framework_network(py::module &m)
{
	using std::vector;
    using std::string;

    using nlohmann::json;

    using neuro::Property;
    using neuro::PropertyPack;
    using neuro::Node;
    using neuro::Edge;
    using neuro::Network;

	py::enum_<Property::Type>(m, "PropertyType")
		.value("Integer", Property::Type::INTEGER)
        .value("Double",  Property::Type::DOUBLE)
        .value("Boolean", Property::Type::BOOLEAN)
        .export_values();

	py::class_<Property>(m, "Property")
		.def(py::init<const std::string&, int, int, double, double, Property::Type>(),
             py::arg("dname"),
             py::arg("idx"),
             py::arg("len"),
             py::arg("dmin"),
             py::arg("dmax"),
             py::arg("dtype"))
		.def(py::init<const json&>(), py::arg("j"))
		.def("to_json", &Property::to_json)
		.def("as_json", &Property::as_json)
		.def("from_json", &Property::from_json)
		.def_readonly("type", &Property::type)
		.def_readonly("index", &Property::index)
		.def_readonly("size", &Property::size)
		.def_readonly("min_value", &Property::min_value)
		.def_readonly("max_value", &Property::max_value)
		.def_readonly("name", &Property::name)
		.def(py::self == py::self)
		.def(py::self != py::self);

	py::class_<PropertyPack>(m, "PropertyPack")
		.def("__call__", [](PropertyPack &self){
			return py::make_tuple(self.nodes, self.edges, self.networks);
		})
		.def("to_json", &PropertyPack::to_json)
		.def("as_json", &PropertyPack::as_json)
		.def("from_json", &PropertyPack::from_json)
		.def("clear", &PropertyPack::clear)
		.def_readonly("nodes", &PropertyPack::nodes)
		.def_readonly("edges", &PropertyPack::edges)
		.def_readonly("networks", &PropertyPack::networks)
		.def_readonly("node_vec_size", &PropertyPack::node_vec_size)
		.def_readonly("edge_vec_size", &PropertyPack::edge_vec_size)
		.def_readonly("net_vec_size", &PropertyPack::net_vec_size)
		.def("add_node_property", &PropertyPack::add_node_property)
		.def("add_edge_property", &PropertyPack::add_edge_property)
		.def("add_network_property", &PropertyPack::add_network_property)
		.def(py::self == py::self)
		.def(py::self != py::self);

	py::class_<Node>(m, "Node")
		.def(py::init<uint32_t, Network*>(), py::arg("idx"), py::arg("n") = nullptr)
		.def_readonly("id", &Node::id)
		.def_readonly("input_id", &Node::input_id)
		.def_readonly("output_id", &Node::output_id)
		.def_readonly("net", &Node::net)
		.def_readonly("values", &Node::values)
		.def_readonly("incoming", &Node::incoming)
		.def_readonly("outgoing", &Node::outgoing)
		.def_readonly("coordinates", &Node::coordinates)
		.def_readonly("name", &Node::name)
		.def("set", py::overload_cast<int, double>(&Node::set))
		.def("set", py::overload_cast<const string&, double>(&Node::set))
		.def("get", py::overload_cast<int>(&Node::get))
		.def("get", py::overload_cast<const string&>(&Node::get))
		.def("is_hidden", &Node::is_hidden)
		.def("is_input", &Node::is_input)
		.def("is_output", &Node::is_output);
	
	py::class_<Edge>(m, "Edge")
		.def(py::init<Node*, Node*, Network*>(), py::arg("f"), py::arg("to"), py::arg("net"))
		.def_readonly("from", &Edge::from)
		.def_readonly("to", &Edge::to)
		.def_readonly("net", &Edge::net)
		.def_readonly("values", &Edge::values)
		.def("as_json", &Edge::as_json)
		.def_readonly("control_point", &Edge::control_point)
		.def("set", py::overload_cast<int, double>(&Edge::set))
		.def("set", py::overload_cast<const string&, double>(&Edge::set))
		.def("get", py::overload_cast<int>(&Edge::get))
		.def("get", py::overload_cast<const string&>(&Edge::get));

	py::class_<Network>(m, "Network")
		.def(py::init<>())
		.def("__eq__", &Network::operator==)
		.def("clear", &Network::clear)
		.def("to_json", &Network::to_json)
		.def("as_json", &Network::as_json)
		.def("from_json", &Network::from_json)
		.def("pretty_json", &Network::pretty_json)
		.def("pretty_nodes", &Network::pretty_nodes)
		.def("pretty_edges", &Network::pretty_edges)
		.def("set_properties", &Network::set_properties)
		.def("get_properties", &Network::get_properties)
		.def("is_node_property", &Network::is_node_property)
		.def("is_edge_property", &Network::is_edge_property)
		.def("is_network_property", &Network::is_network_property)
		.def("get_node_property", &Network::get_node_property)
		.def("get_edge_property", &Network::get_edge_property)
		.def("get_network_property", &Network::get_network_property)
		.def("add_node", &Network::add_node)
		.def("is_node", &Network::is_node)
		.def("get_node", &Network::get_node)
		.def("add_or_get_node", &Network::add_or_get_node)
		.def("remove_node", &Network::remove_node)
		.def("rename_node", &Network::rename_node)
		.def("add_edge", &Network::add_edge)
		.def("is_edge", &Network::is_edge)
		.def("get_edge", &Network::get_edge)
		.def("add_or_get_edge", &Network::add_or_get_edge)
		.def("remove_edge", &Network::remove_edge)

		.def("add_input", &Network::add_input)
		.def("get_input", &Network::get_input)
		.def("add_output", &Network::add_output)
		.def("num_outputs", &Network::num_outputs)

		.def("set_data", &Network::set_data)
		.def("get_data", &Network::get_data)
		.def("data_keys", &Network::data_keys)

		.def("get_random_node", &Network::get_random_node)
		.def("get_random_edge", &Network::get_random_edge)
		.def("get_random_input", &Network::get_random_input)
		.def("get_random_output", &Network::get_random_output)

		.def("randomize_properties", py::overload_cast<neuro::MOA&>(&Network::randomize_properties))
		.def("randomize_properties", py::overload_cast<neuro::MOA&, Node*>(&Network::randomize_properties))
		.def("randomize_properties", py::overload_cast<neuro::MOA&, Edge*>(&Network::randomize_properties))

		.def("randomize_property", py::overload_cast<neuro::MOA&, const string&>(&Network::randomize_property))
		.def("randomize_property", py::overload_cast<neuro::MOA&, Node*, const string&>(&Network::randomize_property))
		.def("randomize_property", py::overload_cast<neuro::MOA&, Edge*, const string&>(&Network::randomize_property))
		.def("randomize_property", py::overload_cast<neuro::MOA&, const Property&, std::vector<double>&>(&Network::randomize_property))

		.def("randomize", &Network::randomize)
		.def("prune", &Network::prune)
		.def("make_sorted_node_vector", &Network::make_sorted_node_vector)
		.def_readonly("sorted_node_vector", &Network::sorted_node_vector)
		.def("begin", &Network::begin)
		.def("end", &Network::end)
		.def("edges_begin", &Network::edges_begin)
		.def("edges_end", &Network::edges_end)

		.def("num_nodes", &Network::num_nodes)
		.def("num_edges", &Network::num_edges)
		.def_readonly("values", &Network::values);
}

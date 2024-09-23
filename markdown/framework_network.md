# The Network class (plus Node and Edge classes)

Point of Contact: James S. Plank

The framework defines a class called `Network`, which holds spiking neural networks.  These
may be created in a variety of ways; however, whenever you create a network, you should use
the methods defined in the `Network` class.  The `Network` class is defined to be very general,
so that the same class may be used by multiple different neuroprocessors.

--------
# Components of a Network

Networks are composed of a few components, which I will go over here.

----------
## A PropertyPack

This defines what values are stored in nodes, edges and the network.  Please read
the [documentation for `PropertyPack`](framework_properties.md) for a description of the
`PropertyPack` and what it does.

----------
## Nodes

A "node" is the same as a "neuron."  We use "node" here to highlight the fact that networks
are really graphs, where neurons are nodes and synapses are edges.  The `Node` class is very
simple, with the following public fields:

- An `id`.  This is a 32-bit unsigned number.  Nodes can have any numbers as id's, and do
  not have to have their id's contiguously numbered.
- An `input_id`.  Networks have input and output nodes, and these are consecutively numbered
  from zero.  If a node is an input node, then this is its index in the input node vector.
  Otherwise, this is -1.
- An `output_id`.  If the node is an output node, then 
  this is set to the index in the output node vector. Otherwise, it is -1.  It is perfectly
  legal for a node to be both an input and an output node.
- A vector of `incoming` edges.
- A vector of `outgoing` edges.
- A vector of doubles called `values`.  These are defined by the `PropertyPack` (see above,
  and see the [documentation for `PropertyPack`](framework_properties.md).
- A vector of doubles called `coordinates`.  Using this vector is optional, but if you want
  to specify coordinates for nodes, for a visualization, then this is where you do it.
- A pointer to the network that contains the node.

You shouldn't set any of these fields, with the exception of `values` and `coordinates.`.
Otherwise, you should use methods in the `Node` and `Network` classes.
Here are the methods of the of the `Node` class:

- `set(index,val)` allows you to set a value in the `values` vector using its index.  I'm not
  sure why you would use this instead of just setting the value in the vector, but hey, that's me.
- `set(name,val)` allows you to set a value by its name in the network's `PropertyPack`.
   That assumes that there is only one value associated with the name (the property's count
   is one).  This error checks the value to make sure it is in the proper range defined by
- `get(index)` returns the value at the index, in case you don't want to access it directly.
- `get(name)` returns the value associated with the `name` in the `PropertyPack`.
- `is_hidden()` returns `true` if the node is neither an input nor an output.
- `is_input()` returns `true` if the node is an input node.
- `is_output()` returns `true` if the node is an output node.


----------
## Edges

These are synapses, defined by the `Edge` class.
Edges are even simpler than nodes, with the following fields:

- A pointer to the `Node` that the edge comes from.  This is `from`.
- A pointer to the `Node` that the edge goes to.  This is `to`.
- A pointer to the network that contains the edge.
- A vector of `values` that are defined by the network's `PropertyPack`.
- A vector of `control_point` values.  These help display the edges if you're
  using a visualizer.  Use of this is optional.

You create and modify edges using methods from the network class.  Edges have `set()` and
`get()` methods that work in the same way as in the `Node` class.

----------
## Values

Just like the nodes and edges, the network itself has a vector of `values` that are defined
by its property pack.

----------
## Associated Data

The network stores a JSON object called its `associated data`.  This contains additional information
about the network, and often about the processor and application for which the network is being
used.  This is used by the `app_agent` so that it knows how to run the network after it has
been trained.  Documentation of the associated data is in [`framework_network_json_format.md`](framework_network_json_format.md).

--------
# Internal Structure of Networks / Workflow to Create and Use a Network

The simplest way to create a network is to simply load one from JSON.

The second simplest way is to use the `processor_tool`, where you create an instance
of a processor, and then use the `EMPTYNET` command to create an empty network for
that processor.  It will have the appropriate `PropertyPack.`

Otherwise, if you're creating
a network from scratch, the first thing to do is set the network's `PropertyPack` with the
`set_property_pack()` method.  How do you define the `PropertyPack?`  Well, typically
you get an instance of the processor that you're using, and call its `get_network_properties()`
method.  Once you have created the first node in the network, you cannot
modify the `PropertyPack`.

You can then add/delete nodes and edges with the various methods in the `Network`.  The
nodes and edges are stored in separate hash tables.  The nodes are keyed by their `id's`, and
edges are keyed by the `id's` of their `from` and `to` nodes.  For that reason, you cannot
have multiedges in the network.  If you need a second edge, then you need to create a new
node and route through it.

You can iterate through the nodes and edges using iterators through the hash table (these
are `NodeMap::iterator` and `EdgeMap::iterator`).  An unfortunate consequence of this is that
the hash table is not sorted.  If you would rather iterate through the nodes sorted by the
node `id`, then call the `make_sorted_node_vector()` method and then you can use the vector
`sorted_node_vector` which is sorted by node id.  There is more documentation on that below.

There are methods to `prune` the network, which deletes nodes and edges that are not on
a path from an input neuron to an output neuron, and randomize various parts of the
network.

--------
# The JSON of a Network

The JSON of a network has the following keys:

- "Nodes" -- this is a vector of Node JSON.  It is not sorted by node id.
- "Edges" -- this is a vector of Edge JSON.  It is not sorted in any way.
- "Inputs" -- this is a vector of the ids of the input neurons.
- "Outputs" -- this is a vector of the ids of the input neurons.
- "Network_Values" -- this is a vector of values defined by the `PropertyPack`
- "Properties" -- this is the `PropertyPack`
- "Associated_Data" -- the associated data JSON (see above)

--------
# Network Methods

## Constructors, Destructors, Etc

- There is a parameterless constructor that creates an empty network.
- There are also copy and move constructors, and the assignment overloads work properly.
- There is a `clear()` method which clears the network to an empty network.  You can
  have it keep the `PropertyPack` or clear it as well.
- There is nothing exciting about the destructor, except that it works.

## JSON

There are the standard `as_json()`, `to_json()` and `from_json` methods.  There are also
three methods that help produce more readable JSON:

- `pretty_json()` returns a formatted string for the entire network.
- `pretty_nodes()` returns a formatted string for the vector of nodes.
         If `make_sorted_node_vector()` has been called and the network hasn't
         changed since that call, then this prints the nodes in sorted order.
         Otherwise, it prints the nodes in the order of the hash table.
- `pretty_edges()` returns a formatted string for the vector of edges.
         They are in no particular order.

## Properties / PropertyPack

- `set_properties()` lets you set the `PropertyPack` for the network.  It will throw
   an exception if the network has any nodes.
- `get_properties()` returns the `PropertyPack` stored in the network.
- `is_node_property(string)`, `is_edge_property(string)` and `is_network_property(string)`
  return bools to state whether the strings are names of properties.
- `get_node_property(string)`, `get_edge_property(string)` and `get_network_property(string)`
  return a pointer to the `Property` associated with the string.

## Adding / modifying / deleting nodes and edges

- `add_node(id)` creates a `Node` with the given `id` and adds it to the network.  This
   throws an exception if there is already a node with `id`.
- `is_node(id)` returns whether or not a node with the given `id` exists.
- `get_node(id)` returns a pointer to the node with the given `id`.  It throws an exception if
   the node doesn't exist.
- `add_or_get_node(id)` returns the node with `id`, if it exists.  If it doesn't exist,
   then it creates a `Node` with the given `id`, adds it to the network, and returns it.
- `remove_node(id, force)` deletes the node with the given `id` from the graph.  If `force`
   is `false`, then it will throw an error if the node is an input or output node.  If `force`
   is `true`, it will delete the node anyway.
- `rename_node(old_id, new_id)` does the necessary machinations to rename a node.

- `add_edge(from_id, to_id)` creates an `Edge` and adds it to the network.  This 
   throws an exception if there is already an edge from `from_id` to `to_id`.
- `is_edge(from_id, to_id)` returns whether there is an edge from `from_id` to `to_id`.
- `get_edge(from_id, to_id)` returns a pointer to the edge from `from_id` to `to_id`.
- `add_or_get_edge(from_id, to_id)` is like `add_or_get_node()`
- `remove_edge(from_id, to_id)` removes the edge.

## Input and output nodes

- `add_input(id)` - This creates a new input and assigns it to the given node.
- `set_input(input_id, node_id)` - This sets the given input node to the node with `node_id`.
   This allows you to replace an input node with another node.  If the input doesn't exist
   it creates it. This is a little dangerous because that means that there may be some
   unassigned input nodes.  
- `get_input(input_id)` returns a `Node` pointer to the given input node.
- `num_inputs()` returns the number of input nodes.

- `add_output()` is analogous to `add_input()`.
- `set_output()` is analogous to `set_input()`.
- `get_output()` is analogous to `get_input()`.
- `num_outputs()` is analogous to `num_outputs()`.

## Associated Data JSON

- `set_data(name, json)` adds a key/val pair to the Associated Data, where the key is `name`
  and the val is `json`.
- `get_data(name)` returns the JSON val associated with the key `name`.  It throws an exception
  if there is no key `name`.
- `data_keys()` returns a vector of the keys in the Associated Data.

## Methods to help do randomization

You pass each of these a `MOA` object, which is a random-number 
generator (see `include/utils/MOA.hpp`).

- `get_random_node()` returns a pointer to a random node.
- `get_random_edge()` returns a pointer to a random edge.
- `get_random_input()` returns a pointer to a random input node.
- `get_random_output()` returns a pointer to a random output node.
- `randomize_properties()` randomizes the `values` vector either in the network, a given
  node or a given edge.  The values are randomized according to their definitions in the
  `PropertyPack`.
- `randomize_property()` randomizes a named property in either the network, a given node
   or a given edge.  You may also use `randomize_property()` to randomize a single property
   in a given `values` vector.
- `randomize()` generates a random network.  Its argument is JSON to parameterize the
   randomization.  Unfortunately, it hasn't been written yet.

## Pruning and Sorting

- `prune()` deletes all nodes and edges that are not on a path from an input to an output.
  There is an optional parameter `prune_io`, which, if `true`, will also prune inputs and
  outputs.  That doesn't seem like a good idea, but it's there.

- `make_sorted_node_vector()` creates `sorted_node_vector`, which is a vector of `Node` pointers
  sorted by node id.  This vector is initialized to be empty, and it is cleared whenever a
  node is added to or removed from the network.  If the vector is non-empty, then
  `make_sorted_node_vector()` does nothing, so you can call it wantonly without any
  performance implications.

## Iterators / Metadata

- `begin()` and `end()` return `NodeMap::iterators` so that you can traverse the nodes in
  the hash table.  If you want to traverse the nodes in sorted order (or you simply want 
  to use a vector rather than the hash table), then use `make_sorted_node_vector()` and
  `sorted_node_vector`.

- `edges_begin()` and `edges_end()` return `EdgeMap::iterators` so that you can traverse the 
  edges in the hash table.   If you want something like `make_sorted_node_vector()` for
  edges, simply pester me and I'll write it.

- `num_nodes()` returns the number of nodes in the network.
- `num_edges()` returns the number of nodes in the network.

----------------
## network_tool

`Network_tool` is a program that gives you a command line interface to all of the
network methods.  See [`utils_network_tool.md`](utils_network_tool.md).

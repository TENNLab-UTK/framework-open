# The Property and PropertyPack data structures.

In order to facilitate generality, we don't require our neuroprocessors to store and 
maintain specific values in their nodes, edges and networks.  For example, a processor
might support neurons with leak, or different modalities of STDP on edges.  Or it might not.

For this reason, we have defined the `Property` and `PropertyPack` classes.  
To summarize:

- A `Property` defines one or more values that may be stored with nodes, edges or networks.
- A `PropertyPack` is a collection of three maps -- one for nodes, one for edges and one for
  networks.  The maps are keyed by property name, and their values are properties.

Processors define `PropertyPacks`, and networks store them.  Each node, edge and network
contains a vector of doubles called `values`.  This vector matches the approprate
map in the `PropertyPack`.

If you are writing a `Processor`, then you'll need to define your `PropertyPack`.
There are methods in the `PropertyPack` class to help you do that.

------------

## Properties and their JSON

A `Property` has the following values, which are all defined in its JSON.  This description
is to help you read `Property` and `PropertyPack` JSON.  Don't bother trying to create this
JSON yourself -- the `Property` and `PropertyPack` methods will do it for you.

| Name        | Type       | Value |
| ----------- | ----       | ----------- |
| `"name"`    | string     | The name of the property (e.g. "leak", "threshold" )
| `"type"`    | char       | 'I' (73) for integer, 'D' (68) for double, 'B' (66) for boolean }
| `"min_value"` | double       | The minimum value for data (for boolean, this should be 0)
| `"max_value"` | double       | The maximum value for data (for boolean, this should be 1)
| `"size"`      | int          | The number of data values that compose this property
| `"index"      | int          | The index where this starts in the `values` vector.

For example, the GNP neuroprocessor can have the following `Property` defined on its nodes:

```
{ "name":"Threshold", "type":68, "index":0, "size":1, "min_value":-1.0, "max_value":1.0 }
```

And the following properties defined on its edges:

```
{ "name":"Weight", "type":68, "index":0, "size":1, "min_value":0.0, "max_value":1.0 }
{ "name":"Inhibitory", "type":66, "index":1, "size":1, "min_value":0.0, "max_value":1.0 }
{ "name":"Delay", "type":73, "index":2, "size":1, "min_value":0.0, "max_value":4.0 }
```

This means that the `values` vector in the nodes contains one value, the Threshold (double).
Edges contain three values -- Weight (double), Inhibitory (boolean) and Delay (integer)

------------

## PropertyPacks and their JSON

A `PropertyPack` is a bundling of Properties.   A neuroprocessor must define three sets of
Properties:

- Properties for nodes -- these are data values that each node will store.
- Properties for edges -- these are data values that each edge will store.
- Properties for networks -- these are data values that each network will store.

Once again, this is to help you read JSON -- don't bother creating it -- instead use the
methods.

The JSON for a `PropertyPack` has three keys:

1. `"node_properties"`: The properties for nodes.
2. `"edge_properties"`: The properties for edges.
3. `"network_properties"`: The properties for networks.

Each of these is an array containing the JSON for the properties.  The properties may be
specified in any order.  However, their indices and sizes must define a contiguous sequence
of values.  Specifically:

- There must be one property whose `index` is zero.
- For each property whose `index` is not zero, there must be a property whose `index+size` equals
  the property's `index`.

For example, here is the default `PropertyPack` JSON for GNP.

```
{ "node_properties": [
    { "name":"Threshold", "type":68, "index":0, "size":1, "min_value":-1.0, "max_value":1.0 }],
  "edge_properties": [
    { "name":"Delay", "type":73, "index":2, "size":1, "min_value":0.0, "max_value":4.0 },
    { "name":"Inhibitory", "type":66, "index":1, "size":1, "min_value":0.0, "max_value":1.0 },
    { "name":"Weight", "type":68, "index":0, "size":1, "min_value":0.0, "max_value":1.0 }],
  "network_properties": [
    { "name":"Enable_Inhibitory_Synapse", "type":73, "index":0, "size":1, "min_value":0.0, "max_value":0.0 }] }
```

## Methods for constructing PropertyPacks

Don't bother with methods from the `Property` class, except for perhaps `as_json()` or `to_json()`.
You can access all of the fields of the `Property` (e.g. `type`, `index`) directly.  Although
they are not `const`, you shouldn't modify them.  This is also true with the `Propery_Pack`
fields.

- `add_node_property()`, `add_edge_property()` and `add_network_property()` create properties
  and add them to the proper map in the `PropertyPack`.  Their arguments are all of the fields
  in the `Property`, with the exception of `index`, because that is calculated automatically.
  For example, the first property in a map will have an `index` of zero.  The next one will
  have an `index` equal to the first property's `count`

That's all there is to it.  There are a few more things:

- `PropertyPack` has a `clear()` method, which clears out the three maps.
- Both classes have `from_json()`, `to_json()` and `as_json()` methods, so that you can 
  store and retrieve them using JSON.
- Both classes have a `pretty_json()` method, which produces a string that is easier to read
  than using the `dump()` method of the JSON.

------------

## Who really needs to care about Properties and PropertyPacks?

I know that this is a little repetitive, but sometimes repetition is good.  If you are exploring
or developing within TENNLab, here's when you may want to care about these data structures:

- You are writing a neuroprocssor.  You will have to define the PropertyPack(s) for your
   neuroprocessor's nodes, edges and networks.  This will be in your processor's `get_properties()`
   method, and will be stored with networks trained with the neuroprocessor.

- Anyone who wants to understand what's in a network.  Nodes, edges and networks all store a
   vector of doubles called `values`.  The definitions of these values are in the `PropertyPack`.
   For example, in GNP, each edge has three values in its vector.  If that vector is 
   `[0.1817,1.0,2.0]`, then you use the `PropertyPack` to interpret it:

   - The edge has a weight of 0.1817.  This is because in the `PropertyPack`, 
          `"Weight"` has an `index` of 0, so it corresponds to the first value: 0.1817.
   - The edge is inhibitory (since that is a boolean - one corresponds to true).
   - The edge has a delay of 2.

   If you are simply exploring networks, then the utility [bin/network_tool](utils_network_tool.md)
   is a good place to start.  It allows you do print out nodes, edges, PropertyPacks, etc on
   network files.  If you are doing something more advanced (for example, writing code to
   visualize a network), then you'll want to use relevant methods in the `Network` class, plus
   of course the `Property` and `PropertyPack` classes.
- The learning or training engine (e.g. EONS).  Let's take EONS as an example.  EONS reads the
   `PropertyPack` so that it knows what parameters it needs to optimize, and what values those
   parameters can take on.

------------
## The property_tool utility program.

The program `bin/property_tool` lets you test and use all of the methods in the `Property` class.
It maintains one instance of a `Property`, which you can manipulate on the command line.  It's
pretty bare bones:

For example:

```
UNIX> bin/property_tool 'PT>'           # Its optional command line argument is a prompt.
PT> ?
For commands that take a json either put a filename on the same line,
or the json can be multiple lines, starting on the next line.

FJ json            - Read an instance of the property class from json.
TJ [file]          - Create JSON from the property.
C n i s t min max  - Create an instance from name index size type min max.
STUFF              - Test the copy constructor, move constructor, assignment.

?                  - Print commands.
Q                  - Quit.
PT> C Fred 4 2 D -14 200                # Create a property from the command line.
PT> TJ                                  # Print its JSON
{ "name":"Fred", "type":68, "index":4, "size":2, "min_value":-14.0, "max_value":200.0 }
PT> FJ                                  # Create a property from JSON
{"index":1,"max_value":1.0,"min_value":0.0,"name":"Inhibitory","size":1,"type":66}
PT> TJ                                  # Print its JSON
{ "name":"Inhibitory", "type":66, "index":1, "size":1, "min_value":0.0, "max_value":1.0 }
PT> TJ tmp.txt                          # Print its JSON to a file.
PT> STUFF                               # Test various constructors, etc.
Passed Test 1
Passed Test 2
Test 3 Done (If there's no other output, that means it passed.
PT> Q
UNIX>                  # You'll note that the JSON in the file is different from the one printed.
UNIX> cat tmp.txt
{"index":1,"max_value":1.0,"min_value":0.0,"name":"Inhibitory","size":1,"type":66}
UNIX> 
```

The reason that the JSON on the screen is different from the JSON printed to the file, is that
the method `pretty_json()` is used to the screen, whereas `to_json.dump()` is used for the file.
That way, the file is more efficiently stored, but you can still read it more clearly.

------------
## The property_pack_tool utility program.

This is analogous to `property_tool`:

```
UNIX> bin/property_pack_tool 'PPT>'
PPT> ?
For commands that take a json either put a filename on the same line,
or the json can be multiple lines, starting on the next line.

FJ json                 - Read a PropertyPack from  json.
TJ [file]               - Create JSON from the property pack.
C                       - Clear.
N name type min max cnt - Add a node property (add_node_property).
E name type min max cnt - Add a edge property (add_edge_property).
W name type min max cnt - Add a network property (add_network_property).

?                       - Print commands.
Q                       - Quit.
PPT> 
PPT> TJ                          # The program starts with an empty property pack.
{ "node_properties": [],
  "edge_properties": [],
  "network_properties": [] }
PPT> N threshold D -10 10 1      # Add a node property called "threshold"
Added: index = 0
PPT> TJ
{ "node_properties": [
    { "name":"threshold", "type":68, "index":0, "size":1, "min_value":-10.0, "max_value":10.0 }],
  "edge_properties": [],
  "network_properties": [] }
PPT> N coordinates D -100 100 3  # Add another node property "coordinates", with three values:
Added: index = 1
PPT> TJ
{ "node_properties": [
    { "name":"coordinates", "type":68, "index":1, "size":3, "min_value":-100.0, "max_value":100.0 },
    { "name":"threshold", "type":68, "index":0, "size":1, "min_value":-10.0, "max_value":10.0 }],
  "edge_properties": [],
  "network_properties": [] }
PPT> TJ tmp.txt                  # When we write the JSON to a file, it will look different from the above printout.
PPT> FJ                          # I did a cut-and-paste from a GNP network file, using bin/network_tool
{ "node_properties": [
    { "name":"Threshold", "type":68, "index":0, "size":1, "min_value":-1.0, "max_value":1.0 }],
  "edge_properties": [
    { "name":"Delay", "type":73, "index":2, "size":1, "min_value":0.0, "max_value":4.0 },
    { "name":"Inhibitory", "type":66, "index":1, "size":1, "min_value":0.0, "max_value":1.0 },
    { "name":"Weight", "type":68, "index":0, "size":1, "min_value":0.0, "max_value":1.0 }],
  "network_properties": [
    { "name":"Enable_Inhibitory_Synapse", "type":73, "index":0, "size":1, "min_value":0.0, "max_value":0.0 }] }
PPT> TJ
{ "node_properties": [
    { "name":"Threshold", "type":68, "index":0, "size":1, "min_value":-1.0, "max_value":1.0 }],
  "edge_properties": [
    { "name":"Delay", "type":73, "index":2, "size":1, "min_value":0.0, "max_value":4.0 },
    { "name":"Inhibitory", "type":66, "index":1, "size":1, "min_value":0.0, "max_value":1.0 },
    { "name":"Weight", "type":68, "index":0, "size":1, "min_value":0.0, "max_value":1.0 }],
  "network_properties": [
    { "name":"Enable_Inhibitory_Synapse", "type":73, "index":0, "size":1, "min_value":0.0, "max_value":0.0 }] }
PPT> E Leak_Rate D 0 10 1             # I'm adding an edge property called "Leak_Rate"
Added: index = 3
PPT> TJ
{ "node_properties": [
    { "name":"Threshold", "type":68, "index":0, "size":1, "min_value":-1.0, "max_value":1.0 }],
  "edge_properties": [
    { "name":"Delay", "type":73, "index":2, "size":1, "min_value":0.0, "max_value":4.0 },
    { "name":"Inhibitory", "type":66, "index":1, "size":1, "min_value":0.0, "max_value":1.0 },
    { "name":"Leak_Rate", "type":68, "index":3, "size":1, "min_value":0.0, "max_value":10.0 },
    { "name":"Weight", "type":68, "index":0, "size":1, "min_value":0.0, "max_value":1.0 }],
  "network_properties": [
    { "name":"Enable_Inhibitory_Synapse", "type":73, "index":0, "size":1, "min_value":0.0, "max_value":0.0 }] }
PPT> Q
UNIX>                                  # Here you see how the file JSON is different.
UNIX> cat tmp.txt
{"edge_properties":[],"network_properties":[],"node_properties":[{"index":1,"max_value":100.0,"min_value":-100.0,"name":"coordinates","size":3,"type":68},{"index":0,"max_value":10.0,"min_value":-10.0,"name":"threshold","size":1,"type":68}]}
UNIX> 
```

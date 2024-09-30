---------
## utils/network_tool.cpp

This is a network tool program that gives you a command-line interface to access, modify,
view and create networks.  Create this with `make bin/network_tool` in the main framework
directory.  If you want to know the commands, simply run it and enter a question mark:

```
UNIX> echo '?' | bin/network_tool
This is a network tool program. The commands listed below are case-insensitive,
For commands that take a json either put a filename on the same line,
or the json can be multiple lines, starting on the next line.

Create/Clear Network Commands
FJ json                    - Read a network.
TJ [file]                  - Create JSON from the network.
COPY_FROM                  - Make a copy of yourself using copy_from.  Print & delete.
DESTROY                    - Delete network, create empty network.
CLEAR                      - Clear network
CLEAR_KP                   - Clear network but keep the property pack intact

Access Network Info Commands
INFO                       - Print some info about the network.
NODES [node_id] [...]      - Print the nodes using a kind of user-friendly JSON format.
EDGES [from] [to] [...]    - Print the edges using a kind of user-friendly JSON format.
PROPERTIES/P               - Print the network's property pack.
ASSOC_DATA                 - Print the network's associated_data.
TYPE node_id  ...          - Print the node's type
NM                         - Print node names map

Network Operation Commands
PRUNE                      - Prune the network - remove nodes/edges not on an I/O path.
AN node_id ...             - Add nodes
AI node_id ...             - Add inputs
AO node_id ...             - Add outputs
AE from to ...             - Add edges 
RN node_id ... [IOE(T|F)]  - Remove nodes. IOE (def:F) flags an error if Input/Output
RE from to ...             - Remove edges
RENAME from to             - Rename nodes
SETNAME node_id name       - Set the names of nodes. A name of "-" clears the name
SETCOORDS node_id [x] [y]  - Set the coordinates of a node. This is used by viz
SET_CP node_id [x] [y]     - Set the control points of a node. This is used by viz
CLEAR_VIZ                  - Get rid of all coordinates and control points in the network
SNP node_id ... name value - Set the node's named property to value.
SEP from to ... name value - Set the edge's named property to value.
SNP_ALL name value         - Set named property to value for all nodes.
SEP_ALL name value         - Set named property to value for all edges.
RNP node_id ... [pname]    - Randomize the node's properties or named property
REP from to ... [pname]    - Randomize the edge's properties or named property
SEED val                   - Seed the RNG
SHOW_SEED                  - Show the RNG seed
RE_SEED                    - Re-seed the RNG based on the current time & show seed
SPROPERTIES/SP json        - Set the network's property pack
SET_ASSOC key json         - Set the key/val in the network's associated data.
SORT/SORTED                - Sort the network and print the sorted node id's

Helper Commands
VIZ T|F [extra_args]       - Open network viz. T|F is to show viz's control panel
RUN                        - Run the app.
?                          - Print commands.
Q                          - Quit.
UNIX> 
```

-----------------------
# Examples of using the network_tool

## 1. Accessing information about a network

First -- if you give it an argument, it will use that as a prompt.  Sometimes that's
useful.  I'll do it here.   We're going to start by taking a look at the 
binary AND network, from the
paper [Spiking Neuromorphic Networks for Binary Tasks](https://neuromorphic.eecs.utk.edu/publications/2021-07-29-spiking-neuromorphic-networks-for-binary-tasks).  Here's ASCII art of the network:

``` 
      W=0.5, D=1
   A -------------|
                  v
                 A&B: Threshold=1
                  ^
   B -------------|
      W=0.5, D=1
``` 

We can create it with `scripts/test_risp.sh`:

```
UNIX> sh scripts/test_risp.sh 1 yes
Passed Test 01 - AND network from [Plank2021], Figure 3.
UNIX> ls -l tmp_network.txt
-rw-r--r--  1 plank  staff  1117 Sep 25 12:48 tmp_network.txt
UNIX> 
```

Now we'll use the `network_tool` to take a look:

```
UNIX> bin/network_tool -                     # We'll use a dash as a prompt.
- FJ tmp_network.txt                         # Read the network -- "FJ" stands for "From JSON"
- INFO                                       # This gives you basic information about a network.
Nodes:          3
Edges:          2
Inputs:         2
Outputs:        1

Input nodes:  0(A) 1(B) 
Hidden nodes: 
Output nodes: 2(A&B) 
- NODES                                      # This shows the JSON of the neurons
[ {"id":0,"name":"A","values":[1.0]},
  {"id":2,"name":"A&B","values":[1.0]},
  {"id":1,"name":"B","values":[1.0]} ]
- EDGES                                      # This shows the JSON of the synapses
[ {"from":0,"to":2,"values":[0.5,1.0]},
  {"from":1,"to":2,"values":[0.5,1.0]} ]
- SORT                                       # You'll note that the neurons aren't sorted.  This sorts them.
0 1 2                                        # It also sorts the synapses, but they are already sorted here.
- NODES
[ {"id":0,"name":"A","values":[1.0]},
  {"id":1,"name":"B","values":[1.0]},
  {"id":2,"name":"A&B","values":[1.0]} ]
-                                            # This shows the properties, which is how we can determine
- P                                          # where the Delays and Weights are stored in the synapses.
{ "node_properties": [
    { "name":"Threshold", "type":68, "index":0, "size":1, "min_value":0.0, "max_value":1.0 }],
  "edge_properties": [
    { "name":"Delay", "type":73, "index":1, "size":1, "min_value":1.0, "max_value":15.0 },
    { "name":"Weight", "type":68, "index":0, "size":1, "min_value":0.0, "max_value":1.0 }],
  "network_properties": [] }
- TJ                                         # TJ prints the network's JSON.  You can give it a filename.
{ "Properties":
  { "node_properties": [
      { "name":"Threshold", "type":68, "index":0, "size":1, "min_value":0.0, "max_value":1.0 }],
    "edge_properties": [
      { "name":"Delay", "type":73, "index":1, "size":1, "min_value":1.0, "max_value":15.0 },
      { "name":"Weight", "type":68, "index":0, "size":1, "min_value":0.0, "max_value":1.0 }],
    "network_properties": [] },
 "Nodes":
  [ {"id":0,"name":"A","values":[1.0]},
    {"id":1,"name":"B","values":[1.0]},
    {"id":2,"name":"A&B","values":[1.0]} ],
 "Edges":
  [ {"from":0,"to":2,"values":[0.5,1.0]},
    {"from":1,"to":2,"values":[0.5,1.0]} ],
 "Inputs": [0,1],
 "Outputs": [2],
 "Network_Values": [],
 "Associated_Data":                             # You'll note that the network file stores information
   { "other": {"proc_name":"risp"},             # about the neuroprocessor on which it runs.
     "proc_params": 
      { "discrete": false,
        "fire_like_ravens": false,
        "leak_mode": "all",
        "max_delay": 15,
        "max_threshold": 1.0,
        "max_weight": 1.0,
        "min_potential": 0.0,
        "min_threshold": 0.0,
        "min_weight": 0.0,
        "run_time_inclusive": false,
        "spike_value_factor": 1.0,
        "threshold_inclusive": true}}}
- Q
UNIX>
```

------------------------------
## 2. Modifying a network

Let's suppose that we want to change the network above to an OR network:

``` 
      W=1, D=1
   A -------------|
                  v
                 A|B: Threshold=1
                  ^
   B -------------|
      W=1, D=1
``` 

We can do it with some `network_tool` commands:

```
UNIX> mv tmp_network.txt AND.txt
UNIX> bin/network_tool -
- FJ AND.txt
- NODES                                     # Take a look at the nodes and edges.
[ {"id":0,"name":"A","values":[1.0]},
  {"id":2,"name":"A&B","values":[1.0]},
  {"id":1,"name":"B","values":[1.0]} ]
- EDGES
[ {"from":0,"to":2,"values":[0.5,1.0]},
  {"from":1,"to":2,"values":[0.5,1.0]} ]
- SETNAME 2 A|B                             # Change the name of the output neuron to A|B
- SEP 0 2  1 2  Weight 1                    # Change the weights of the two edges to 1.
- NODES                                     # Confirm that everything looks right
[ {"id":0,"name":"A","values":[1.0]},
  {"id":2,"name":"A|B","values":[1.0]},
  {"id":1,"name":"B","values":[1.0]} ]
- EDGES
[ {"from":0,"to":2,"values":[1.0,1.0]},
  {"from":1,"to":2,"values":[1.0,1.0]} ]
- TJ OR.txt                                 # Write it to the file OR.txt
- Q
UNIX> 
```

------------------------------
## 2. Creating a network from scratch

Let's suppose that we want to create a network from scratch.  How about this
XOR network, again from 
[Spiking Neuromorphic Networks for Binary Tasks](https://neuromorphic.eecs.utk.edu/publications/2021-07-29-spiking-neuromorphic-networks-for-binary-tasks).  This is the network in Figure 5, which
lets you do XOR on integrate-and-fire neurons, like RISP's networks when leak is disabled.

``` 
   A       B
   |\     /|
   | \   / |     Thresholds are 1
   |  \ /  |     Delays are 1
   |   X   |
   |  / \  |     A->H2, B->H1, have weights of -1 
   | /   \ |
   |/     \|
   v       v
   H1 <-> H2
   |       |
   |       |
   \-> O <-/
``` 

We can use the `network_tool` to create this network.  It's a little cumbersome, because you
neet to specify the properties of the neurons and synapses, like threshold, delay and weight.
The easiest way to do that is to create an empty network for a specific neuroprocessor with
the `processor_tool`.  Let's do that.  The RISP-1 setting of RISP has the properties that
we need:

```
UNIX> cat params/risp_1.txt
{ 
  "min_weight": -1,
  "max_weight": 1,
  "leak_mode": "none",
  "min_threshold": 0,
  "max_threshold": 1,
  "min_potential": -1,
  "max_delay": 15,
  "discrete": true
}
UNIX> 
```

The `EMPTYNET` command of the `processor_tool` creates an empty network for a given neuroprocessor,
so we'll use that to create an empty network:

```
UNIX> bin/processor_tool_risp -
- M risp params/risp_1.txt              # "Make" a neuroprocessor with the given parameters.
- EMPTYNET tmp-empty.txt                # Create an empty network for that neuroprocessor
- Q
UNIX> 
```

Now we can use the `network_tool` to create our neurons and synapses for this processor:

UNIX> bin/network_tool -
- FJ tmp-empty.txt                     # Start with the empty network we just created.
- P                                    # Take a look at the properties -- we see Threshold, Delay and Weight.
{ "node_properties": [
    { "name":"Threshold", "type":73, "index":0, "size":1, "min_value":0.0, "max_value":1.0 }],
  "edge_properties": [
    { "name":"Delay", "type":73, "index":1, "size":1, "min_value":1.0, "max_value":15.0 },
    { "name":"Weight", "type":73, "index":0, "size":1, "min_value":-1.0, "max_value":1.0 }],
  "network_properties": [] }
- AN 0 1 2 3 4                         # Add the five neurons
- SETNAME 0 A                          # Set their names.  This is optional, but often useful.
- SETNAME 1 B
- SETNAME 2 H1
- SETNAME 3 H2
- SETNAME 4 A^B
- AI 0 1                               # Assign 0 and 1 as inputs.
- AO 4                                 # Assign 4 as an output.
- INFO                                 # Take a look at what we have so far:
Nodes:          5
Edges:          0
Inputs:         2
Outputs:        1

Input nodes:  0(A) 1(B) 
Hidden nodes: 2(H1) 3(H2) 
Output nodes: 4(A^B) 
- NODES
[ {"id":0,"name":"A","values":[1.0]},
  {"id":4,"name":"A^B","values":[1.0]},
  {"id":1,"name":"B","values":[1.0]},
  {"id":2,"name":"H1","values":[1.0]},
  {"id":3,"name":"H2","values":[1.0]} ]
- 
-
- AE 0 2  0 3  1 2  1 3  2 3  3 2  2 4  3 4        # Now, add the edges. 
- EDGES                                            # When we look at them, they are unsorted, which
[ {"from":1,"to":3,"values":[1.0,15.0]},           # is unfortunate.  Moreover, when they were created,
  {"from":1,"to":2,"values":[1.0,15.0]},           # they were assigned the maximum values that they can be.
  {"from":0,"to":2,"values":[1.0,15.0]},           # That's fine for most of the weights, but those
  {"from":2,"to":4,"values":[1.0,15.0]},           # delays are all wrong.
  {"from":0,"to":3,"values":[1.0,15.0]},           
  {"from":2,"to":3,"values":[1.0,15.0]},
  {"from":3,"to":4,"values":[1.0,15.0]},
  {"from":4,"to":3,"values":[1.0,15.0]},
  {"from":3,"to":2,"values":[1.0,15.0]} ]
- SORT                                             # Let's sort them so that they look better.
0 1 2 3 4
- EDGES                                            # They look better, but we have to fix those delays
[ {"from":0,"to":2,"values":[1.0,15.0]},           # and two of those weights.
  {"from":0,"to":3,"values":[1.0,15.0]},
  {"from":1,"to":2,"values":[1.0,15.0]},
  {"from":1,"to":3,"values":[1.0,15.0]},
  {"from":2,"to":3,"values":[1.0,15.0]},
  {"from":2,"to":4,"values":[1.0,15.0]},
  {"from":3,"to":2,"values":[1.0,15.0]},
  {"from":3,"to":4,"values":[1.0,15.0]},
  {"from":4,"to":3,"values":[1.0,15.0]} ]
- SEP_ALL Delay 1                                  # SEP_ALL is pretty handy -- it changes all of the delays to 1 here.
- SEP 0 3  1 2  Weight -1                          # This changes the weights of the two synapses.
- EDGES 
[ {"from":0,"to":2,"values":[1.0,1.0]},
  {"from":0,"to":3,"values":[-1.0,1.0]},
  {"from":1,"to":2,"values":[-1.0,1.0]},
  {"from":1,"to":3,"values":[1.0,1.0]},
  {"from":2,"to":3,"values":[1.0,1.0]},
  {"from":2,"to":4,"values":[1.0,1.0]},
  {"from":3,"to":2,"values":[1.0,1.0]},
  {"from":3,"to":4,"values":[1.0,1.0]},
  {"from":4,"to":3,"values":[1.0,1.0]} ]
- TJ XOR.txt                                       # We write it to XOR.txt.
- Q
UNIX>
```

Let's look at the whole network -- you should be able to identify everything here.

```
UNIX> cat XOR.txt
{ "Properties":
  { "node_properties": [
      { "name":"Threshold", "type":73, "index":0, "size":1, "min_value":0.0, "max_value":1.0 }],
    "edge_properties": [
      { "name":"Delay", "type":73, "index":1, "size":1, "min_value":1.0, "max_value":15.0 },
      { "name":"Weight", "type":73, "index":0, "size":1, "min_value":-1.0, "max_value":1.0 }],
    "network_properties": [] },
 "Nodes":
  [ {"id":0,"name":"A","values":[1.0]},
    {"id":1,"name":"B","values":[1.0]},
    {"id":2,"name":"H1","values":[1.0]},
    {"id":3,"name":"H2","values":[1.0]},
    {"id":4,"name":"A^B","values":[1.0]} ],
 "Edges":
  [ {"from":0,"to":2,"values":[1.0,1.0]},
    {"from":0,"to":3,"values":[-1.0,1.0]},
    {"from":1,"to":2,"values":[-1.0,1.0]},
    {"from":1,"to":3,"values":[1.0,1.0]},
    {"from":2,"to":3,"values":[1.0,1.0]},
    {"from":2,"to":4,"values":[1.0,1.0]},
    {"from":3,"to":2,"values":[1.0,1.0]},
    {"from":3,"to":4,"values":[1.0,1.0]},
    {"from":4,"to":3,"values":[1.0,1.0]} ],
 "Inputs": [0,1],
 "Outputs": [4],
 "Network_Values": [],
 "Associated_Data":
   { "other": {"proc_name":"risp"},
     "proc_params": 
      { "discrete": true,
        "fire_like_ravens": false,
        "leak_mode": "none",
        "max_delay": 15,
        "max_threshold": 1.0,
        "max_weight": 1.0,
        "min_potential": -1.0,
        "min_threshold": 0.0,
        "min_weight": -1.0,
        "run_time_inclusive": false,
        "spike_value_factor": 1.0,
        "threshold_inclusive": true}}}
UNIX> 
```

If you want more information about the JSON specs of networks, please see
[network_json_format.md](network_json_format.md).

------------------------------
# Shell scripting (and python programs)

By now, I hope that you can see that the `network_tool` lends itself very nicely to
shell scripting and python.  It's a very simple matter to write programs that output `network_tool`
commands so you can create networks.  This is exactly how the 
[DBSCAN rep](https://github.com/TENNLab-UTK/dbscan) generates its networks.

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

## Usage

1. Access network info
```
UNIX> bin/network_tool ':'                      # The command line argument is a prompt.
: FJ utils/networks/caspian_network.json
: INFO
Please use "VIZ" option to visually see network if you have love installed

Nodes:         16    # This is the number of nodes
Edges:         17
Inputs:         8
Outputs:        3

Input nodes:  0   1   2   3   4   5   6   7   # Node ids
Hidden nodes: 13  12  11  34  14  
Output nodes: 8   9   10  
: NODES             # print all nodes info
[ {"id":0,"values":[9.0,-1.0,0.0]},
  {"id":6,"values":[116.0,-1.0,0.0]},
  {"id":13,"values":[88.0,-1.0,0.0]},
  {"id":12,"values":[111.0,-1.0,0.0]},
  {"id":8,"values":[78.0,-1.0,0.0]},
  {"id":7,"values":[125.0,-1.0,0.0]},
  {"id":11,"values":[56.0,-1.0,0.0]},
  {"id":2,"values":[37.0,-1.0,0.0]},
  {"id":3,"values":[83.0,-1.0,0.0]},
  {"id":9,"values":[26.0,-1.0,0.0]},
  {"id":4,"values":[88.0,-1.0,0.0]},
  {"id":34,"values":[55.0,-1.0,0.0]},
  {"id":1,"values":[100.0,-1.0,0.0]},
  {"id":10,"values":[20.0,-1.0,0.0]},
  {"id":14,"values":[30.0,-1.0,0.0]},
  {"id":5,"values":[37.0,-1.0,0.0]} ]
: NODES 0 6 1000    # print nodes 0,6 and 1000 info
{"id":0,"values":[9.0,-1.0,0.0]}
{"id":6,"values":[116.0,-1.0,0.0]}
Node 1000 does not exist.
: EDGES             # print edges info
[ {"from":2,"to":10,"values":[45.0,6.0]},
  {"from":13,"to":9,"values":[81.0,8.0]},
  {"from":4,"to":5,"values":[-116.0,8.0]},
  {"from":12,"to":8,"values":[0.0,7.0]},
  {"from":4,"to":12,"values":[53.0,13.0]},
  {"from":5,"to":10,"values":[123.0,6.0]},
  {"from":8,"to":5,"values":[49.0,7.0]},
  {"from":14,"to":8,"values":[-123.0,14.0]},
  {"from":0,"to":8,"values":[-18.0,4.0]},
  {"from":2,"to":2,"values":[126.0,5.0]},
  {"from":4,"to":13,"values":[116.0,7.0]},
  {"from":3,"to":9,"values":[53.0,8.0]},
  {"from":7,"to":10,"values":[64.0,14.0]},
  {"from":12,"to":10,"values":[-54.0,9.0]},
  {"from":8,"to":7,"values":[12.0,9.0]},
  {"from":12,"to":13,"values":[-115.0,14.0]},
  {"from":3,"to":8,"values":[40.0,11.0]} ]
: EDGES 2 10  13 9  0 1000    # print edges (2->10), (13->9) and (0->1000) info.
{"from":2,"to":10,"values":[45.0,6.0]}
{"from":13,"to":9,"values":[81.0,8.0]}
Edge 0 -> 1000 does not exist.
: PROPERTIES                  # print property pack info
{ "node_properties": [
    { "name":"Delay", "type":73, "index":2, "size":1, "min_value":0.0, "max_value":0.0 },
    { "name":"Leak", "type":73, "index":1, "size":1, "min_value":-1.0, "max_value":-1.0 },
    { "name":"Threshold", "type":73, "index":0, "size":1, "min_value":0.0, "max_value":127.0 }],
  "edge_properties": [
    { "name":"Delay", "type":73, "index":1, "size":1, "min_value":0.0, "max_value":15.0 },
    { "name":"Weight", "type":73, "index":0, "size":1, "min_value":-127.0, "max_value":127.0 }],
  "network_properties": [] }
: TYPE 0 1 1000              # print nodes type
0 is an input node
1 is an input node
Node 1000 does not exist.
: 
```
2. Create a network from scratch. If you have network viz open, you will see the real-time change as you create the network. Let's create a fully connected network with 3 layers where each layer has 4 nodes.
```
UNIX> bin/network_tool "NT>"
NT> SP     # set the network property pack first.
{ "node_properties": [{ "name":"Threshold", "type":73, "index":0, "size":1, "min_value":0.0, "max_value":127.0 }],
  "edge_properties": [{ "name":"Weight", "type":73, "index":0, "size":1, "min_value":-127.0, "max_value":127.0 }],
  "network_properties": [] }
NT> AN 0 1 2 3   4 5 6 7   8 9 10 11  # add 12 nodes
NT> AI 0 1 2 3                        # add 4 inputs
NT> AO 8 9 10 11                      # add 4 outputs
NT> AE 0 4  0 5  0 6   0 7       1 4  1 5  1 6   1 7       2 4  2 5  2 6   2 7       3 4  3 5  3 6  3 7     
# add edges between first and second layer
NT> AE 4 8  4 9  4 10  4 11      5 8  5 9  5 10  5 11      6 8  6 9  6 10  6 11      7 8  7 9  7 10  7 11
# add edges between second and third layer
NT> INFO
Please use "VIZ" option to visually see network if you have love installed

Nodes:         12
Edges:         32
Inputs:         4
Outputs:        4

Input nodes:  0   1   2   3   
Hidden nodes: 6   7   4   5   
Output nodes: 8   9   10  11  
NT> VIZ F
NT> NODES
[ {"id":0,"values":[0.0]},
  {"id":6,"values":[0.0]},
  {"id":8,"values":[0.0]},
  {"id":7,"values":[0.0]},
  {"id":11,"values":[0.0]},
  {"id":2,"values":[0.0]},
  {"id":3,"values":[0.0]},
  {"id":9,"values":[0.0]},
  {"id":4,"values":[0.0]},
  {"id":1,"values":[0.0]},
  {"id":10,"values":[0.0]},
  {"id":5,"values":[0.0]} ]
NT> SNP  0 1 2 3   Threshold  120  # set "Threshold" property value to 120
NT> RNP  4 5 6 7 8 9 10 11         # randomize all node properties. In this case, we only have "Threshold" property.
NT> NODES 0 1 2 3
{"id":0,"values":[120.0]}
{"id":1,"values":[120.0]}
{"id":2,"values":[120.0]}
{"id":3,"values":[120.0]}
NT> NODES 4 5 6 7 8 9 10 11        # you may get different values due to different rng seeds.
{"id":4,"values":[55.0]}
{"id":5,"values":[115.0]}
{"id":6,"values":[82.0]}
{"id":7,"values":[87.0]}
{"id":8,"values":[48.0]}
{"id":9,"values":[119.0]}
{"id":10,"values":[14.0]}
{"id":11,"values":[0.0]}
NT> EDGES
[ {"from":3,"to":6,"values":[0.0]},
  {"from":7,"to":8,"values":[0.0]},
  {"from":6,"to":11,"values":[0.0]},
  {"from":3,"to":5,"values":[0.0]},
  {"from":5,"to":10,"values":[0.0]},
  {"from":3,"to":4,"values":[0.0]},
  {"from":1,"to":6,"values":[0.0]},
  {"from":6,"to":10,"values":[0.0]},
  {"from":1,"to":5,"values":[0.0]},
  {"from":4,"to":8,"values":[0.0]},
  {"from":5,"to":11,"values":[0.0]},
  {"from":7,"to":9,"values":[0.0]},
  {"from":4,"to":9,"values":[0.0]},
  {"from":6,"to":8,"values":[0.0]},
  {"from":0,"to":7,"values":[0.0]},
  {"from":1,"to":7,"values":[0.0]},
  {"from":2,"to":4,"values":[0.0]},
  {"from":1,"to":4,"values":[0.0]},
  {"from":6,"to":9,"values":[0.0]},
  {"from":0,"to":4,"values":[0.0]},
  {"from":2,"to":7,"values":[0.0]},
  {"from":7,"to":10,"values":[0.0]},
  {"from":0,"to":5,"values":[0.0]},
  {"from":5,"to":9,"values":[0.0]},
  {"from":4,"to":11,"values":[0.0]},
  {"from":3,"to":7,"values":[0.0]},
  {"from":0,"to":6,"values":[0.0]},
  {"from":2,"to":6,"values":[0.0]},
  {"from":4,"to":10,"values":[0.0]},
  {"from":7,"to":11,"values":[0.0]},
  {"from":5,"to":8,"values":[0.0]},
  {"from":2,"to":5,"values":[0.0]} ]
NT> SEP 0 4  0 5  0 6   0 7       1 4  1 5  1 6   1 7       2 4  2 5  2 6   2 7       3 4  3 5  3 6  3 7   Weight 20
# set "Weight" property value to 20 for edges between input and hidden layers.
NT> REP 4 8  4 9  4 10  4 11      5 8  5 9  5 10  5 11      6 8  6 9  6 10  6 11      7 8  7 9  7 10  7 11  
# randomize all properties for edges betwwen hidden and output layer. In this case, we only have "Weight" property.
NT> EDGES 0 4  0 5  0 6   0 7      
{"from":0,"to":4,"values":[20.0]}
{"from":0,"to":5,"values":[20.0]}
{"from":0,"to":6,"values":[20.0]}
{"from":0,"to":7,"values":[20.0]}
NT> EDGES 4 8  4 9  4 10  4 11    # you may get different values due to different rng seeds.
{"from":4,"to":8,"values":[0.0]}
{"from":4,"to":9,"values":[64.0]}
{"from":4,"to":10,"values":[-3.0]}
{"from":4,"to":11,"values":[14.0]}
NT> RENAME 7 20    # rename node 7 to node 20
NT> RENAME 20 7    # change it back
NT> AN 20 21 22
NT> AE 20 21   21 22   22 20   # create a loop among 20, 21 and 22.
NT> PRUNE F        # node 20, 21 and 22 are removed.
NT> AN 20          # add node 20
NT> AI 20          # make node 20 input node
NT> RN 20          # try to remove node 20
Input node 20 cannot be removed. 
NT> RN 20 T        # force to remove input node 20
```

I find it difficult to deal with node ids as numbers since they are meaningless. 
The good thing is you don't have to. We internally map the string value 
to the node id.
```
UNIX> bin/network_tool
$: SP
{ "node_properties": [
    { "name":"Threshold", "type":68, "index":0, "size":1, "min_value":-1.0, "max_value":2.0 }],
  "edge_properties": [
    { "name":"Delay", "type":73, "index":1, "size":1, "min_value":1.0, "max_value":5.0 },
    { "name":"Weight", "type":68, "index":0, "size":1, "min_value":-1.0, "max_value":1.0 }],
  "network_properties": [] }
$: AN A B C D # Add node A, B, C, and D
$: NM A B C D
"A": 0    # value is the node id.
"B": 1
"C": 2
"D": 3
$: NM
{"A":0,"B":1,"C":2,"D":3}
```

3. Run the app as we edit the network the way we think it can improve the fitness score.
```
UNIX> bin/network_tool "NT>"
NT> RUN
app name is not specified in the network
NT> FJ utils/networks/caspian_network.json
NT> VIZ F
NT> PRUNE F
NT> RUN
running polebalance_caspian
Fitness 16
NT> AE 0 10
NT> SEP 0 10 Weight 50
NT> RUN
running polebalance_caspian
Fitness 80
NT> AE 1 9 
NT> SEP 1 9 Weight 30
NT> RUN
running polebalance_caspian
Fitness 103
NT> AE 6 10
NT> SEP 6 10 Weight 70
NT> RUN
running polebalance_caspian
Fitness 48
NT> 
```


---------
## The processor_tool

This is a program that gives you a command-line interface to load networks onto a
neuroprocessor, apply inputs, run them examine outputs and examine processor state.
This markdown file is sketchy, because the main documentation on how to use the
processor_tool is in the [getting started walkthrough](getting_started.md), the
[network_tool walkthrough](network_tool.md), and the [RISP walkthrough](risp.md).

If you want to know the commands, simply run the tool and enter a question mark:

```
UNIX> echo '?' | bin/processor_tool_risp 
This is a processor tool program. The commands listed below are case-insensitive,
For commands that take a json either put a filename on the same line,
or the json can be multiple lines, starting on the next line.

Action commands --
MAKE/M proc_name proc_param_json    - Make a new processor with no network
LOAD/L network_json                 - Load a network on the processor
ML network_json                     - Make a new processor from the network & load the network.
AS node_id spike_time spike_val ... - Apply normalized spikes to the network (note: node_id, not input_id)
ASV node_id spike_time spike_val .. - Apply unnormalized spikes to the network (note: node_id, not input_id)
ASR node_id spike_raster_string     - Apply spike raster to the network (note: node_id, not input_id)
RUN simulation_time                 - Run the network for "simulation_time" cycles
RSC/RUN_SR_CH sim_time [node] [...] - Run, and then print spike raster and charge information in columns
CLEAR-A/CA                          - Clear the network's internal state 
CLEAR/C                             - Remove the network from processor

Output tracking info commands --
OLF [node_id] [...]                 - Output the last fire time for the given output or all outputs
OC [node_id] [...]                  - Output the spike count for the given output or all outputs
OT/OV [node_id] [...]               - Output the spike times for the given output or all outputs
TRACK_O [node_id] [...]             - Track output events for given outputs (empty=all)
UNTRACK_O [node_id] [...]           - Untrack output events for given output (empty=all)

Neuron / synapse tracking info commands --
NLF show_nonfiring(T/F)             - Last fire times for neurons.
NC show_nonfiring(T/F)              - Fire counts for neurons.
TNC                                 - Total fire count for all neurons.
TNA                                 - Total accumulates for all neurons.
NT/NV show_nonfiring(T/F)           - All firing times for tracked neurons.
GSR [T/F]] [node] [...]             - Print spike raster info for tracked neurons.
NCH [node_id] [...]                 - Print the charges (action potentials) of the neurons (empty=all).
NLFJ                                - Print the neuron last fire json
NCJ                                 - Print the neuron count json
NVJ type(V/S)                       - Print the neuron vector json
NCHJ                                - Print the neuron charge json
TRACK_N [node_id] [...]             - Track neuron events for specified neurons (empty=all)
UNTRACK_N [node_id] [...]           - Untrack neuron events for specified neurons (empty=all)
SW [from to]                        - Show synapse weights (or just one synapse).
PULL_NETWORK file                   - Pull the network off the processor and store in  file.

Other info commands --
PARAMS [file]                       - Print the JSON that can recreate the processor
NP/PPACK                            - Print the PropertyPack that networks use with the processor.
NAME                                - Print the processor's name
EMPTYNET [file]                     - Create an empty network for this processor
PP                                  - Print processor's properties - (not a universal feature).
INFO                                - Print the network's node ids and output tracking info
PS                                  - Print the spikes we have applied
?                                   - Print commands
Q                                   - Quit
UNIX> 
```

------------------------------
# Shell scripting (and python programs)

The `processor_tool` lends itself very nicely to
shell scripting and python.  It's a very simple matter to write programs that output `processor_tool`
commands so you can automate neuromorphic applications.  The
[DBSCAN repo](https://github.com/TENNLab-UTK/dbscan) does this so you can see how the algorithm
runs neuromorphically.

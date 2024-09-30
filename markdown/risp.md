# RISP - Reduced Instruction Spiking Processor

Point of contact: James S. Plank

------

[TOC]

------

# Introduction

RISP is a lightweight neuroprocessor. It has the following features. 

  1. It implements a standard integrate-and-fire neuron with discrete integration
     timesteps.  In other words, neurons accumulate their action potentials during a
     timestep, and at the end of the timestep, they check the potential and fire if it
     meets or exceeds the threshold.  For that reason, when we define RISP networks, the
     synapses have discrete, integer delays.
  2. Weights, thresholds and activation potentials may be set to integer values
     (`"discrete" = true`), or floating point values (`"discrete" = false`).
  3. Neurons have two leak modes: `"none"`, where the neurons do not leak, and `"all"`,
     where the neurons leak all of their potential at the end of every timestep.  The
     RISP neuroprocessor may be set so that all neurons have the same leak value, or so
     that each neuron may set its value individually.
  4. There's no plasticity on the synapse.
  5. There are no refractory periods or learning rules.  RISP is simple.
  6. However, it does have a few "features" which have been very helpuful in research.  The
     first is that you may affix specific weight values for the synapses.  This is independent
     of whether `"discrete"` is `true` or `false`.
  7. You may also random noise to every synapse fire.
  8. There are parameter settings that allow RISP to emulate specific parameter settings of
     the RAVENs neuroprocessor.  

Please read ["The Case for RISP: A Reduced Instruction Spiking Processor"](http://neuromorphic.eecs.utk.edu/publications/2022-06-29-the-case-for-risp-a-reduced-instruction-spiking-processor/) for a precise description of RISP.

# Implementations

This directory contains the implementation of the RISP simulator.

There is an open-source FPGA implementation of RISP at
[https://github.com/TENNLab-UTK/fpga](https://github.com/TENNLab-UTK/fpga).

There is a version of RISP that compiles networks onto a RP2040 Pico microcontroller.
You can find that in 
[The Embedded Neuromorphic Repository](https://bitbucket.org/neuromorphic-utk/embedded-neuromorphic), but you'll need bitbucket access from TENNLab, because this implementation is not open-source.
It may be someday, but not yet.

# ------------------------------------------------------------
# Default RISP Parameter Settings

There are three main classes of RISP:

1. Floating point: `RISP-F` and `RISP-F+`.
2. Discrete with positive and negative weights: `RISP-n`
3. Discrete with positive weights only: `RISP-n+`

In the `params` directory, there are parameter files for many of these RISP defaults:

```
UNIX> ls params | cat
risp_1.txt
risp_127.txt
risp_15_plus.txt
risp_1_plus.txt
risp_255_plus.txt
risp_7.txt
risp_f.txt
risp_f_plus.txt
UNIX> 
```

All of these parameter settings have `"leak_mode"` set to `"none"`.  If you want to enable leak,
then copy the file you want, edit it so that `"leak_mode"` is `"all"` or `"configurable"`, and
you're ready to go.

`RISP-F` and `RISP-F+` are straightforward.  You can just look at the JSON to see the settings.

Our RISP FPGA and microcontrollers do not implement floating point, 
so RISP-F and RISP-F+ networks cannot run on the FPGA or microcontroller.  For those, you should
use a `RISP-n` or `RISP-n+` setting.
`RISP-n` means that:

- Maximum synapse weight is *n*.
- Minimun synapse weight is *-n*.
- Maximum neuron threshold is *n*.
- Minimum neuron threshold is *0*.
- Minimum neuron potential is *-n*.
- Maximum synapse delay is *max(n,15)*.

`RISP-n+1` means that:

- Maximum synapse weight is *n*.
- Minimun synapse weight is *1*.
- Maximum neuron threshold is *n*.
- Minimum neuron threshold is *1*.
- Minimum neuron potential is *0*.
- Maximum synapse delay is *max(n,15)*.

Now you should understand all of the files in the `params` directory, and you can select the one
to use for your task.  The smaller values of *n* are more efficiently implemented on FPGA.






------------------------------------------------------------
# Params

You can specify the following parameters:


| Key                 | Type   | Default      | Description |
|-------------------- |------- | ------------ |------------ |
| discrete            | bool   | Necessary    | If `true`, weights, thresholds and activation potentials are integers.  Otherwise they are floating point. (Default param file: `false`) |
| min_weight          | double | Necessary*   | The minimum synapse weight (Default param file: -1).  Not required when `weights` are specified. |
| max_weight          | double | Necessary*   | The maximum synapse weight (Default param file: 1).  Not required when `weights` are specified. |
| min_threshold       | double | Necessary    | The minimum neuron threshold (Default param file: -1) |
| max_threshold       | double | Necessary    | The maximum neuron threshold (Default param file: 1) |
| max_delay           | int    | Necessary    | The maximum synapse delay (Default param file: 5) |
| min_potential       | double | Necessary    | At the end of integration, the potential cannot go lower than this value. (Default param file: -1) |
| leak_mode           | string | `"none"`     | Leak: `"all"`, `"none"`, `"configurable"` |
| run_time_inclusive  | bool   | `false`      | If `true`, `run(duration)` calls run for `duration+1` timesteps. |
| threshold_inclusive | bool   | `true`       | If `true` neurons spike when the potential is >= the threshold.  If `false`, then it has to exceed the threshold. |
| fire_like_ravens    | bool   | `false`      | If `true` neurons fire on the timestep after they normally fire.  The synapses work as normal. |
| spike_value_factor  | double | `max_weight` | Framework applications call `apply_spikes()` with input spike values between 0 and 1.  RISP multiplies these values by this factor. |
| weights             | vector | []           | If this is specified, then synapse weights are restricted to these values.  If `discrete` is true, these must be integers. |
| inputs_from_weights | bool   | Necessary*   | Must specify if you use `weights`.  If `true`, then input spikes map to the `weights` array.  Otherwise, `spike_value_factor` is used. |
| noisy_seed          | int    | 0            | If noise is used (either `noisy_stddev` or `stds` is specified), then this is the RNG seed. 0 uses the current time in microseconds. |
| noisy_stddev        | double | 0            | A random normal with this standard deviation is added to the weight on each synapse fire. ||
| stds                | vector | []           | Each time a synapse with `weights[i]` fires, a random normal with `stds[i]` is added to/subtracted from the weight. |
| log                 | JSON   | {}           | IO_Stream to log events (for debugging) | 

------------------------------------------------------------
# Examples of Use

I'm going to use the `jsp_test_network.sh` script to generate network that we'll walk
through with the `processor_tool`.  You may read about the script
[here](../../markdown/scripts_jsp_test_network.md).  

You should be in the `cpp-apps` directory, and you should have `network_tool`
and `processor_tool_risp` compiled:

```
UNIX> pwd
/Users/plank/src/repos/framework/cpp-apps
UNIX> ( cd .. ; make bin/network_tool )
...
UNIX> make app=processor_tool proc=risp
...
UNIX> 
```

----------------------------------------
## Test 1: RISP with defaults.

For this test, we use RISP defaults and set all of the values to 1:

```
UNIX> sh ../scripts/jsp_test_network.sh 1 1 1 1 1 1 1 risp params/risp.json > tmp.txt
```

Our network is the one pictured below.
In this and later pictures, all weights, delays and thresholds are one unless otherwise
specified.  

![jpg/network_1_all_ones.jpg](jpg/network_1_all_ones.jpg)

Let's use the network tool to see the neuron id numbers that correspond to the various
neurons:

```
UNIX> ../bin/network_tool -
- FJ tmp.txt
- NODES  
[ {"id":0,"name":"Main","values":[1.0]},           # Main is neuron 0
  {"id":4,"name":"Bias","values":[1.0]},           # Bias is neuron 4
  {"id":1,"name":"On","values":[1.0]},             # On is neuron 1
  {"id":2,"name":"Off","values":[1.0]},            # Off is neuron 2
  {"id":3,"name":"Out","values":[1.0]} ]           # Out is neuron 3
- Q
UNIX> 
```

Now, we'll run the processor tool to make sure we understand what is happening when this
network runs.  Please read the commentary that is inline:

```
UNIX> bin/processor_tool_risp -                     # Our prompt will be a single dash.
- ML tmp.txt                                        # Create the processor and load our network.
- AS 0 0 1                                          # Send an input spike at time zero to the Main neuron
- RUN 10                                            # Run for ten timesteps.
- GSR
                                   # Here's the spike raster.  The input spike causes the Main
                                   # neuron to fire every timestep.  That in turn causes the 
                                   # Out and Bias neurons to spike every timestep:
0(Main) INPUT  : 1111111111
1(On)   INPUT  : 0000000000
2(Off)  INPUT  : 0000000000
3(Out)  OUTPUT : 0111111111
4(Bias) OUTPUT : 0111111111

- RUN 5                            # When we run for another five timesteps, we see that the
- GSR                            # Main, Out and Bias neurons continue their spiking.
0(Main) INPUT  : 11111
1(On)   INPUT  : 00000
2(Off)  INPUT  : 00000
3(Out)  OUTPUT : 11111
4(Bias) OUTPUT : 11111
- GT                               # This says how many timesteps the neuroprocessor has run,
time: 15.0                         # since it was created (or cleared).

- AS 2 0 1                         # We send input spikes to the Off neuron at timestep 0 (relative
- AS 1 5 1                         # to the current timestep), and to the On neuron at timestep 5.
- RUN 10 
- GSR
  
                                   # The Off neuron fires at timestep 0, canceling the spike on
                                   # synapse C at tiemstep 1.  Accordingly, the Out neuron
                                   # stops firing at timestep 2.  
                                   # 
                                   # At timestep 5, the "On" neuron fires, causing the Main neuron 
                                   # to start firing at timestep 6.  This in turn causes the Out
                                   # neuron to start firing at timestep 7.
0(Main) INPUT  : 1000001111
1(On)   INPUT  : 0000010000
2(Off)  INPUT  : 1000000000
3(Out)  OUTPUT : 1100000111
4(Bias) OUTPUT : 1111111111
- GT                               # The processor has been running for 25 timesteps.
time: 25.0

- AS 1 0 1   2 0 1                 # We have the On and Off neurons both spike at timestep 0.
- RUN 5
- GSR

                                   # Since they both spike at timestep 0, their spikes arrive at
                                   # the Main neuron at timestep 1.  They cancel each other out,
                                   # So the Main neuron continues to fire.
0(Main) INPUT  : 11111
1(On)   INPUT  : 10000
2(Off)  INPUT  : 10000
3(Out)  OUTPUT : 11111
4(Bias) OUTPUT : 11111
- Q
```


----------------------------------------
## Test 2: Setting some different delays to see neurons "in flight"

This is a trivial test, but I want you to see that spikes can be "in flight" when
the *RUN* call terminates, and they'll be delivered at the proper timestep in a 
subsequent *RUN* call.  We use the following network:

```
UNIX> sh ../scripts/jsp_test_network.sh 4 5  1 1 1 1 1 risp params/risp.json > tmp.txt
```

![jpg/network_2_delays.jpg](jpg/network_2_delays.jpg)

Again, please read the commentary inline:
```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 1   2 0 1               # We apply spikes to the Main and Off neurons at time zero.
- RUN 10                         # This "turns on" the Main and Bias neurons, and then turns the
- GSR                          # Main neuron off at timestep 5.  One quirk of the "GSR" command
0(Main) INPUT  : 1111100000      # is that if no neurons spike in the last timesteps, then it won't
1(On)   INPUT  : 0000000000      # show those timesteps.  That's why I have the Bias neuron, so that
2(Off)  INPUT  : 1000000000      # you can see all of the timesteps, even when there's no activity
3(Out)  OUTPUT : 0111110000      # in the other neurons.
4(Bias) OUTPUT : 0111111111

- AS 1 0 1   2 0 1               # We apply spikes to the On and Off neurons simultaneously.
- RUN 3                          # The "On" spike will arrive in 4 timesteps, and the "Off" spike
- GSR                          # will arrive in 5 timesteps.  However, we only run for three
0(Main) INPUT  : 000             # timesteps.  When the *RUN* call completes, those two spikes
1(On)   INPUT  : 100             # are in flight on synapses A and B.
2(Off)  INPUT  : 100
3(Out)  OUTPUT : 000
4(Bias) OUTPUT : 111
- RUN 5                          # When we run again, the spikes are delivered properly.
- GSR
0(Main) INPUT  : 01000
1(On)   INPUT  : 00000
2(Off)  INPUT  : 00000
3(Out)  OUTPUT : 00100
4(Bias) OUTPUT : 11111
- Q
```

----------------------------------------
## Test 3: Putting an extra delay on Main's self-loop synapse

We next set all parameters to 1, except we put a delay of two on the synapse from
*Main* to itself:

```
UNIX> sh ../scripts/jsp_test_network.sh 1 1   2   1 1 1 1 risp params/risp.json > tmp.txt
```

![jpg/network_3_self_synapse.jpg](jpg/network_3_self_synapse.jpg)

The end result here is to have the *Main* neuron fire every other cycle, which in turn has
the *Out* neuron fire every other cycle:

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 1
- RUN 10
- GSR
0(Main) INPUT  : 1010101010
1(On)   INPUT  : 0000000000
2(Off)  INPUT  : 0000000000
3(Out)  OUTPUT : 0101010101
4(Bias) OUTPUT : 0111111111
- 
```

Here's something fun -- we apply a spike to the *On* neuron at timestep zero, which spikes
the *Main* neuron at time 1.  Now, the *Main* neuron starts firing at every timestep, because
at each timestep it has a spike in flight on Synapse C.

```
- AS 1 0 1 
- RUN 10
- GSR
0(Main) INPUT  : 1111111111
1(On)   INPUT  : 1000000000
2(Off)  INPUT  : 0000000000
3(Out)  OUTPUT : 0111111111
4(Bias) OUTPUT : 1111111111
-    
```
We can use the *Off* neuron to cancel one of the spikes, and now the *Main* neuron goes
back to spiking every other timestep:

```
- AS 2 0 1
- RUN 10
- GSR
0(Main) INPUT  : 1010101010
1(On)   INPUT  : 0000000000
2(Off)  INPUT  : 1000000000
3(Out)  OUTPUT : 1101010101
4(Bias) OUTPUT : 1111111111
- Q
UNIX> 
```

----------------------------------------
## Test 4: Setting The Weight of the Synapse to the Out neuron to 0.5

Now we put all of the delays back to 1, and set the weight of the synapse from *Main*
to *Out* to 0.5:
```
UNIX> sh ../scripts/jsp_test_network.sh 1 1 1 1 1   0.5   1 risp params/risp.json > tmp.txt
```

![jpg/network_4_half_weight.jpg](jpg/network_4_half_weight.jpg)

Now, it takes two spikes from *Main* to cause *Out* to spike.  Thus, it spikes every
other timestep:

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 1
- RUN 10
- GSR
0(Main) INPUT  : 1111111111
1(On)   INPUT  : 0000000000
2(Off)  INPUT  : 0000000000
3(Out)  OUTPUT : 0010101010
4(Bias) OUTPUT : 0111111111
- 
```

Lets spike the *Off* neuron.  This stops the *Main* neuron from spiking.  However, there is a spike
sent from *Main* to *Out* at timestep zero, which arrives at the *Out* neuron at timestep 1.  This
means that at the end of the *RUN* call, the *Out* neuron's activation potential is 0.5.
We can confirm that with the *NCH* call, that shows each neuron's action potential:

```
- AS 2 0 1
- RUN 10
- GSR
0(Main) INPUT  : 1000000000
1(On)   INPUT  : 0000000000
2(Off)  INPUT  : 1000000000
3(Out)  OUTPUT : 1000000000   # At timestep 1, the Out neuron receives 0.5 charge from Main
4(Bias) OUTPUT : 1111111111
- NCH
Node 0(Main) charge: 0
Node   1(On) charge: 0
Node  2(Off) charge: 0
Node  3(Out) charge: 0.5      # This is confirmed here.
Node 4(Bias) charge: 0
- 
```

We'll use the *On* and *Off* neurons to have Main send one more spike -- that causes
the *Out* neuron to fire:

```
- AS 1 0 1   2 1 1             # Apply a spike to On at time 0, and to Off at time 1
- RUN 10
- GSR
0(Main) INPUT  : 0100000000    # That causes Main to spike exactly once.
1(On)   INPUT  : 1000000000
2(Off)  INPUT  : 0100000000
3(Out)  OUTPUT : 0010000000    # And the one spike gives Out its final 0.5 of charge, so it fires
4(Bias) OUTPUT : 1111111111
- Q
UNIX> 
```

----------------------------------------
## Test 5: Turning On Leak

RISP has a parameter setting called `leak_mode`.  It can have three values:

1. `"none"` - No leak (this is the default, and what I've used above).
2. `"all"` - Every neuron leaks its potential to zero at the end of every timestep.
3. `"configurable"` - You can set each neuron's leak to `true', meaning it leaks its
    potential every timestep, or `false`, meaning it doesn't leak.

I have set `leak_mode` to `"all"` in the 
parameter file [`params/risp_leak.json`](params/risp_leak.json):

```
UNIX> cat params/risp_leak.json
{
  "min_weight": -1,
  "max_weight": 1, 
  "min_threshold": -1,
  "max_threshold": 1,
  "min_potential": -1,
  "max_delay": 5,
  "discrete": false,
  "leak_mode": "all"
}
```

Now, let's create the same network as in the last test, but
now our neurons leak their charge at the end of each timestep.  As such, the *Out* neuron
never gets enough charge, and it never fires:

```
UNIX> sh ../scripts/jsp_test_network.sh 1 1 1 1 1   0.5   1 risp params/risp_leak.json > tmp.txt
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 1
- RUN 10
- GSR
0(Main) INPUT  : 1111111111
1(On)   INPUT  : 0000000000
2(Off)  INPUT  : 0000000000
3(Out)  OUTPUT : 0000000000     # Out never fires, because it leaks its charge at every timestep.
4(Bias) OUTPUT : 0111111111
- Q
UNIX> 
```

With `leak_mode` set to `"all"`, all of the neurons leak their charge.  
So, for example, if we don't
put the full amount of charge into the input neurons, they won't fire:

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 1  2 0 1          # Start up the Bias, and turn the Main neuron off:
- RUN 5
- GSR
0(Main) INPUT  : 10000
1(On)   INPUT  : 00000
2(Off)  INPUT  : 10000
3(Out)  OUTPUT : 00000
4(Bias) OUTPUT : 01111
- AS 0 0 .5   0 1 .5   1 0 .99  1 1 .99    # Put partial charge into Main and On at timesteps
- RUN 5                                    # 0 and 1.  Nothing will happen
- GSR
0(Main) INPUT  : 00000
1(On)   INPUT  : 00000
2(Off)  INPUT  : 00000
3(Out)  OUTPUT : 00000
4(Bias) OUTPUT : 11111
- NCH
Node 0(Main) charge: 0                     # As you can see, they have no charge.
Node   1(On) charge: 0
Node  2(Off) charge: 0
Node  3(Out) charge: 0
Node 4(Bias) charge: 0
- Q
UNIX> 
```

----------------------------------------
## Test 6: Using Configurable Leak in RISP

RISP allows you to set leak on individual neurons.  To do that, you set the `leak_mode`
to `"configurable"`.  I've done that
in [`params/risp_prog_leak.json`](params/risp_prog_leak.json):

```
UNIX> cat params/risp_prog_leak.json
{ 
  "min_weight": -1,
  "max_weight": 1, 
  "min_threshold": -1,
  "max_threshold": 1,
  "max_delay": 5,
  "min_potential": -1,
  "discrete": false,
  "leak_mode": "configurable"                    # Here is the setting
}

UNIX> 
```

We'll set up the same network as before, but use the programmable leak parameter file:

```
UNIX> sh ../scripts/jsp_test_network.sh 1 1 1 1 1  0.5  1 risp params/risp_prog_leak.json > tmp.txt
```

![jpg/network_4_half_weight.jpg](jpg/network_4_half_weight.jpg)

When we run this below, you'll see that the neurons still leak all of
their charge.  That is because the `network_tool` sets all parameters to their maximum values
when it creates neurons and synapses.  When `leak_mode` is `"configurable"`, each neuron
has a `"Leak"` property which is a boolean.  0 is false and 1 is true, so the `network_tool`
sets all neurons `"Leak"` properties to 1 -- true.

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 1
- RUN 10
- GSR
0(Main) INPUT  : 1111111111
1(On)   INPUT  : 0000000000
2(Off)  INPUT  : 0000000000
3(Out)  OUTPUT : 0000000000    # Out doesn't spike, because it leaks away its charge
4(Bias) OUTPUT : 0111111111
- Q
UNIX> 
```

We can use the `network_tool` to confirm the leak values:

```
UNIX> ../bin/network_tool -
- FJ tmp.txt
- P                      # You can see the Leak is the second property of a node (because index=1).
{ "node_properties": [
    { "name":"Leak", "type":66, "index":1, "size":1, "min_value":0.0, "max_value":1.0 },
    { "name":"Threshold", "type":68, "index":0, "size":1, "min_value":-1.0, "max_value":1.0 }],
  "edge_properties": [
    { "name":"Delay", "type":73, "index":1, "size":1, "min_value":1.0, "max_value":5.0 },
    { "name":"Weight", "type":68, "index":0, "size":1, "min_value":-1.0, "max_value":1.0 }],
  "network_properties": [] }
- NODES
[ {"id":0,"name":"Main","values":[1.0,1.0]},       # All nodes have thresholds of 1, and leaks of 1
  {"id":4,"name":"Bias","values":[1.0,1.0]},
  {"id":1,"name":"On","values":[1.0,1.0]},
  {"id":2,"name":"Off","values":[1.0,1.0]},
  {"id":3,"name":"Out","values":[1.0,1.0]} ]
- Q
UNIX> 
```

Let's now use the network tool to set the *Out* neuron's Leak to `false`.  

```
UNIX> ../bin/network_tool -
- FJ tmp.txt
- NODES
[ {"id":0,"name":"Main","values":[1.0,1.0]},
  {"id":4,"name":"Bias","values":[1.0,1.0]},
  {"id":1,"name":"On","values":[1.0,1.0]},
  {"id":2,"name":"Off","values":[1.0,1.0]},
  {"id":3,"name":"Out","values":[1.0,1.0]} ]            # The Out neuron is neuron #3
- SNP 3 Leak 0                                          # SNP stands for Set Neuron Property
- NODES
[ {"id":0,"name":"Main","values":[1.0,1.0]},
  {"id":4,"name":"Bias","values":[1.0,1.0]},
  {"id":1,"name":"On","values":[1.0,1.0]},
  {"id":2,"name":"Off","values":[1.0,1.0]},
  {"id":3,"name":"Out","values":[1.0,0.0]} ]            # You can see its value has been changed.
- TJ tmp.txt
- Q
UNIX> 
```

Here's the network pictorally:

![jpg/network_5_prog_leak.jpg](jpg/network_5_prog_leak.jpg)

When we run this, the *Out* neuron goes back to firing every other timestep, because
it retains its potential from timestep to timestep:

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 1
- RUN 10
- GSR
0(Main) INPUT  : 1111111111
1(On)   INPUT  : 0000000000
2(Off)  INPUT  : 0000000000
3(Out)  OUTPUT : 0010101010              # Out now fires every other timestep
4(Bias) OUTPUT : 0111111111
- NCH
Node 0(Main) charge: 0
Node   1(On) charge: 0
Node  2(Off) charge: 0
Node  3(Out) charge: 0.5                 # You can see that at this point, Out has non-zero charge.
Node 4(Bias) charge: 0
- Q
UNIX> 
```

--------------------------------------------------
## Test 7: Run_time_inclusive

RISP has a parameter `run_time_inclusive`.  By default, it is set to `false`.  If you
set it to `true`, then each `RUN` call goes for an extra timestep.  Why do we have this?
Well, some processors (e.g. RAVENS) do this in hardware, 
so we thought it would be a good thing to match it.

Below, we'll simply use a `sed` script to set it from its default to `true` and create
our base network:

```
UNIX> sed '/discrete/s/$/, "run_time_inclusive": true/' params/risp.json > tmp-params.txt
UNIX> cat tmp-params.txt
{
  "min_weight": -1,
  "max_weight": 1,
  "min_threshold": -1,
  "max_threshold": 1,
  "max_delay": 5,
  "discrete": false, "run_time_inclusive": true          # Here's the new setting.
}
UNIX> sh ../scripts/jsp_test_network.sh 1 1 1 1 1 1 1 risp tmp-params.txt > tmp.txt
UNIX> 
UNIX> 
```

Here's the network:

![jpg/network_1_all_ones.jpg](jpg/network_1_all_ones.jpg)

When we call `RUN 5`, you'll see that it actually runs for 6 timesteps:

```
UNIX> bin/processor_tool_risp -
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 1
- RUN 5
- GSR
0(Main) INPUT  : 111111           # There are six timesteps rather than five
1(On)   INPUT  : 000000
2(Off)  INPUT  : 000000
3(Out)  OUTPUT : 011111
4(Bias) OUTPUT : 011111
- GT                              # The processor's timer also says 6 rather than five.
time: 6.0
- RUN 5
- GT
time: 12.0
- Q
UNIX> 
```

--------------------------------------------------
## Test 8: threshold_inclusive

Another optional parameter is `threshold_inclusive`, which specifies whether neurons fire
when their potential meets/exceeds the threshold, or whether they only fire when their
potential exceeds the threshold.  The default is `true`, which means they fire when the
potential meets or exceeds the threshold.  You can see that pretty easily in the examples
above.  

Let's demonstrate this by creating our base network with `threshold_inclusive` set to `false`:

```
UNIX> sed '/discrete/s/$/, "threshold_inclusive": false/' params/risp.json > tmp-params.txt
UNIX> sh ../scripts/jsp_test_network.sh 1 1 1 1 1 1 1 risp tmp-params.txt > tmp.txt
UNIX> 
```

When we run it and apply a spike with a value of 1, the *Main* neuron doesn't fire:

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 1
- RUN 5
- GSR
0(Main) INPUT  :      # If there are no spikes, GSR doesn't print anything
1(On)   INPUT  : 
2(Off)  INPUT  : 
3(Out)  OUTPUT : 
4(Bias) OUTPUT : 
- 
```

If we spike in a small value, that will cause the *Main* neuron to fire, 
but since all of the synapse weights are 1, none of the spikes will cause
the other neurons to fire:

```
- AS 0 0 0.01
- RUN 5
- GSR
0(Main) INPUT  : 1
1(On)   INPUT  : 0
2(Off)  INPUT  : 0
3(Out)  OUTPUT : 0
4(Bias) OUTPUT : 0
-
```

If we want our network to work like it did before, we can change all of the neuron 
potentials to 0.99:

```
UNIX> ../bin/network_tool -
- FJ tmp.txt
- SNP 0 1 2 3 4 Threshold 0.99               # This sets all thresholds to 0.99
- NODES
[ {"id":0,"name":"Main","values":[0.99]},
  {"id":4,"name":"Bias","values":[0.99]},
  {"id":1,"name":"On","values":[0.99]},
  {"id":2,"name":"Off","values":[0.99]},
  {"id":3,"name":"Out","values":[0.99]} ]
- TJ tmp.txt
- Q
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 1
- RUN 5
- GSR
0(Main) INPUT  : 11111           # Now it runs like before.
1(On)   INPUT  : 00000
2(Off)  INPUT  : 00000
3(Out)  OUTPUT : 01111
4(Bias) OUTPUT : 01111
- Q
UNIX> 
```


--------------------------------------------------
## Test 9: Discrete = true

The RISP parameter `discrete` is a necessary parameter.  If `true`, then all parameters -- 
thresholds, delays and weights -- must be integers.  Let's take a look at 
a parameter file where `discrete` is set to `true`:

```
UNIX> cat params/risp_discrete.json
{ 
  "min_weight": -10,                      # You'll note that weights and thresholds are larger,
  "max_weight": 10,                       # since they are taking on discrete values.
  "min_threshold": 0,
  "max_threshold": 10,
  "min_potential": -10,
  "max_delay": 5,
  "discrete": true
}
UNIX> 
```

And we'll set up our default network where the threshold of the *Out* neuron is three:

```
UNIX> sh ../scripts/jsp_test_network.sh  1 1 1 1 1 1 3 risp params/risp_discrete.json > tmp.txt
```

![jpg/network_6_discrete.jpg](jpg/network_6_discrete.jpg)

When we put a spike into the main neuron, as expected, the *Out* neuron fires every third
timestep:

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 1
- RUN 12
- GSR
0(Main) INPUT  : 111111111111
1(On)   INPUT  : 000000000000
2(Off)  INPUT  : 000000000000
3(Out)  OUTPUT : 000100100100        # The *Out* neuron fires every third cycle
4(Bias) OUTPUT : 011111111111
- NCH
Node 0(Main) charge: 0
Node   1(On) charge: 0
Node  2(Off) charge: 0
Node  3(Out) charge: 2               # Its potential is 2 at this point.
Node 4(Bias) charge: 0
- Q
UNIX> 
```

--------------------------------------------------
## Test 10: spike_value_factor

As mentioned several times already in this README, the framework's `apply_spikes()` method
specifies spike values between 0 and 1.  It is up to each neuroprocessor to convert these
values correctly.  What RISP does is use the parameter `spike_value_factor`, which it
multiplies by the spike value from `apply_spikes()`.  If `discrete` is `true`, then the
floor of this value is taken, and that is the value of the spike.

If `spike_value_factor` is not specified, then it is set to the maximum synapse
weight (the parameter `max_weight`).  Let's explore this a little.  We'll use the
same network as above: 

```
UNIX> sh ../scripts/jsp_test_network.sh  1 1 1 1 1 1 3 risp params/risp_discrete.json > tmp.txt
```

![jpg/network_6_discrete.jpg](jpg/network_6_discrete.jpg)

When I called 
`AS 0 0 1` above in the `processor_tool`, it really put a value of 10 into the input neuron.  
Of course,
the neuron spiked.  However, if we call `AS 0 0 0.1`, it will still spike, because the
`spike_value_factor` is 10:

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 0.1
- RUN 12
- GSR
0(Main) INPUT  : 111111111111
1(On)   INPUT  : 000000000000
2(Off)  INPUT  : 000000000000
3(Out)  OUTPUT : 000100100100
4(Bias) OUTPUT : 011111111111
- Q
UNIX> 
```

If you call `AS 0 0 0.09`, then `floor(10*0.09) = 0` and the input neuron won't spike:

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 0.09
- RUN 12
- GSR
0(Main) INPUT  :                  # No spikes
1(On)   INPUT  : 
2(Off)  INPUT  : 
3(Out)  OUTPUT : 
4(Bias) OUTPUT : 
- NCH
Node 0(Main) charge: 0            # No charge in the Main neuron, because floor(0.09*10) = 0
Node   1(On) charge: 0
Node  2(Off) charge: 0
Node  3(Out) charge: 0
Node 4(Bias) charge: 0
- Q
UNIX> 
```

Let's go back to the RISP default, where `discrete` is `false`, and set `spike_value_factor`
to 0.5.  We'll set up a network where all of the synapse weights are 1 or -1:

```
UNIX> sed '/min_weight/s/$/ "spike_value_factor": 0.5,/' params/risp.json > tmp-risp.json
UNIX> cat tmp-risp.json
{
  "min_weight": -1, "spike_value_factor": 0.5,
  "max_weight": 1,
  "min_threshold": -1,
  "max_threshold": 1,
  "min_potential": -1,
  "max_delay": 5,
  "discrete": false
}
UNIX> sh ../scripts/jsp_test_network.sh 1 1 1 1 1 1 1 risp tmp-risp.json > tmp.txt
```

![jpg/network_1_all_ones.jpg](jpg/network_1_all_ones.jpg)

And now when we apply a spike whose value is one, it really spikes with a value of 0.5:

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 1
- RUN 10
- GSR
0(Main) INPUT  :         # No spikes, because the spike value was 0.5
1(On)   INPUT  : 
2(Off)  INPUT  : 
3(Out)  OUTPUT : 
4(Bias) OUTPUT : 
- NCH
Node 0(Main) charge: 0.5         # There it is.
Node   1(On) charge: 0
Node  2(Off) charge: 0
Node  3(Out) charge: 0
Node 4(Bias) charge: 0
- Q
UNIX>
```

--------------------------------------------------
## Test 11:  min_potential

This parameter specifies the minimum potential value that a neuron may store.  You'll note that
while integrating (processing spikes), the potential may go below this value, but at the end of 
integration, if the potential is less than this value, then it is set to the value.

I'm going to use the default RISP settings, where `min_potential` is zero, and we'll set up
the following network, where all delays and thresholds are one:

![jpg/network_B_min_potential.jpg](jpg/network_B_min_potential.jpg)

You can ignore this if you want, but here's how I make the network the the `network_tool`
and `processor_tool`:

```
UNIX> cat params/risp.json
{
  "min_weight": -1,
  "max_weight": 1,
  "min_threshold": -1,
  "max_threshold": 1,
  "min_potential": -1,
  "max_delay": 5,
  "discrete": false
}
UNIX> bin/processor_tool_risp -
- M risp params/risp.json                # First, create an empty network with the processor_tool
- EMPTYNET tmp.txt
- Q
UNIX> ../bin/network_tool -              # Now set nodes and edges with the network_tool:
- FJ tmp.txt                             # Start with the empty network
- AN 0 1 2 3 4                           # Add nodes 0 through 4
- AI 0 1 2 3 4                           # Set the nodes as input nodes.
- AE 0 4  1 4  2 4  3 4                  # Add the four edges, all to node 4
- SEP 1 4 Weight -0.6                    # Set the weights as in the picture
- SEP 2 4 Weight -0.5
- SEP 3 4 Weight -0.2
- SEP 0 4 Delay 1                        # Set the edge delays to one (they would be 5 otherwise)
- SEP 1 4 Delay 1
- SEP 2 4 Delay 1
- SEP 3 4 Delay 1
- SORT
0 1 2 3 4
- NODES                                  # Confirm that everything is as in the picture.
[ {"id":0,"values":[1.0]},
  {"id":1,"values":[1.0]},
  {"id":2,"values":[1.0]},
  {"id":3,"values":[1.0]},
  {"id":4,"values":[1.0]} ]
- EDGES
[ {"from":0,"to":4,"values":[1.0,1.0]},
  {"from":1,"to":4,"values":[-0.6,1.0]},
  {"from":2,"to":4,"values":[-0.5,1.0]},
  {"from":3,"to":4,"values":[-0.2,1.0]} ]
- TJ tmp.txt
- Q
UNIX> 
```

Now, let's run it in a variety of ways.  First, let's cause neuron 1 to spike twice.  Although
the two weights are -0.6, neuron 4's potential is set to its minimum value, -1, rather than
the sum of the weights, -1.2:

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 1 0 1   1 1 1   
- RUN 5
- NCH
Node 0 charge: 0
Node 1 charge: 0
Node 2 charge: 0
Node 3 charge: 0
Node 4 charge: -1               # The value is -1 rather than -1.2, because of min_potential.
- Q
UNIX> 
```

Let's instead have neurons 0, 1 and 2 spike at the same time.  The charge on neuron 4 will
be -0.1, because 1 - 0.6 - 0.5 = -0.1.  This is always the case -- we don't worry about
the order of the spikes or that (-0.6)+(-0.5) goes below `min_potential`.  The only thing
that matters is its value at the end of the integration cycle:

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 1   1 0 1   2 0 1
- RUN 3
- NCH
Node 0 charge: 0
Node 1 charge: 0
Node 2 charge: 0
Node 3 charge: 0
Node 4 charge: -0.1
- Q
UNIX> 
```

Just to hammer this point home further, I'm going to have neuron 3 spike four times, which will
set neuron 4's potential to -0.8.  Then I'll have 0, 1 and 2 spike, and you'll see that the value
becomes -0.9, because -0.8 + 1 - 0.6 - 0.5 = -0.9.  It doesn't matter in which order I integrate
the spikes.

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 3 0 1  3 1 1  3 2 1  3 3 1   
- AS 0 4 1  1 4 1  2 4 1
- RUN 6
- NCH
Node 0 charge: 0
Node 1 charge: 0
Node 2 charge: 0
Node 3 charge: 0
Node 4 charge: -0.9      # Here's value of -0.9.
- Q
UNIX> 
```

--------------------------------------------------
## Test 12: noisy_stddev

With `noisy_stddev`, you can add noise to every synapse fire.  To demonstrate this, I'm going
to set up the following network: 

![jpg/network_C_noisy_stddev.jpg](jpg/network_C_noisy_stddev.jpg)

I'll start neuron 0 firing at time 0 and neuron 1 firing at time 1.  That way, neuron 0 fires
on the even cycles and neuron 1 fires on the odd cycles.  I'll set up RISP so that a random
number with standard deviation 0.01 gets added to the synapse weights:

```
UNIX> sed '/min_weight/s/$/ "noisy_stddev": 0.01,/' params/risp.json > tmp-risp.json
UNIX> cat tmp-risp.json
{
  "min_weight": -1, "noisy_stddev": 0.01,
  "max_weight": 1,
  "min_threshold": -1,
  "max_threshold": 1,
  "min_potential": -1,
  "max_delay": 5,
  "discrete": false
}
UNIX>
```
And I'll create the network with the `processor_tool` and `network_tool`:

```
UNIX> bin/processor_tool_risp -
- M risp tmp-risp.json
- EMPTYNET tmp.txt
- Q
UNIX> ../bin/network_tool -
- FJ tmp.txt
- AN 0 1 2
- AI 0 1
- AE 0 0   1 1   0 2   1 2
- SEP 0 0 1 1 Delay 2
- SEP 0 2 1 2 Delay 1  
- SNP 0 1 Threshold 0
- SNP 2 Threshold 0.5
- SEP 0 2 Weight 0.1              # The other weights default to one.
- NODES
[ {"id":0,"values":[0.0]},        # Confirm that the network is like the picture above.
  {"id":2,"values":[0.5]},
  {"id":1,"values":[0.0]} ]
- EDGES
[ {"from":0,"to":0,"values":[1.0,2.0]},
  {"from":1,"to":1,"values":[1.0,2.0]},
  {"from":1,"to":2,"values":[1.0,1.0]},
  {"from":0,"to":2,"values":[0.1,1.0]} ]
- TJ tmp.txt
- Q
UNIX> 
```

Now I run the processor tool and set nodes 0 and 1 spiking at alternating timesteps.  You'll
note that node 2 also starts firing at even timesteps.  That's because on odd timesteps, it gets
the spike from node 0, which has a weight of *0.1+r*, where *r* is a random normal with a standard
deviation of 0.1.  Whatever the spike's value is, it's too small to make node 2 spike.  However,
then the spike comes in from node 1, and again, regardless of its actual value, it *will* be big
enough to make node 2 spike.  Hence the alternation:

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 1   1 1 1
- RUN 20
- GSR
0 INPUT  : 10101010101010101010
1 INPUT  : 01010101010101010101
2 HIDDEN : 00101010101010101010
- Q  
UNIX> 
```

Now, let's see those random numbers.  What I'll do is print node 2's charge value at every timestep:

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 1   1 1 1
- RUN 1
- NCH 2
Node 2 charge: 0
- RUN 1
- NCH 2
Node 2 charge: 0.100533            # Here's the first spike from node 0
- RUN 1
- NCH 2
Node 2 charge: 0                   # The spike from node 1 causes node 2 to fire and reset.
- RUN 1
- NCH 2
Node 2 charge: 0.102426            # And here's the second spike from node 0
- Q
UNIX> 
```

Can we prove to ourselves that the random numbers are indeed normals with a standard deviation
of 0.01?  Sure -- the following shell command will record 10,000 spike values from node zero:

```
UNIX> ( echo ML tmp.txt ; echo AS 0 0 1 1 1 1; i=0; while [ $i -lt 10000 ]; do echo RUN 2; echo NCH 2; i=$(($i+1)); done ) | bin/processor_tool_risp > tmp-spikes.txt
UNIX> head tmp-spikes.txt
Node 2 charge: 0.0984676
Node 2 charge: 0.0872493
Node 2 charge: 0.0878994
Node 2 charge: 0.105179
Node 2 charge: 0.0935409
Node 2 charge: 0.107039
Node 2 charge: 0.106671
Node 2 charge: 0.0972982
Node 2 charge: 0.106364
Node 2 charge: 0.104167
UNIX> 
```

We'll graph a histogram -- looks good enough for me!

![jpg/histo.jpg](jpg/histo.jpg)

You'll note that during each run, the random charge values are different:

```
UNIX> ( echo ML tmp.txt ; echo AS 0 0 1 1 1 1; echo RUN 2 ; echo NCH 2) | bin/processor_tool_risp 
Node 2 charge: 0.103169
UNIX> ( echo ML tmp.txt ; echo AS 0 0 1 1 1 1; echo RUN 2 ; echo NCH 2) | bin/processor_tool_risp 
Node 2 charge: 0.0869593
UNIX> ( echo ML tmp.txt ; echo AS 0 0 1 1 1 1; echo RUN 2 ; echo NCH 2) | bin/processor_tool_risp 
Node 2 charge: 0.0963038
UNIX> 
```

That is because by default, the RNG seed is 0, which means to calculate it from the current time.
If we instead set the `noisy_seed`, we'll get consistent values.  I'm going to do that by setting
the `proc_params` in the network file, and then you'll see that the charge values are always
generated in the same way:

```
UNIX> ed tmp.txt                       # I'm adding "noisy_seed": 1 to the processor parameters in the network
1199
/noisy_std
        "noisy_stddev": 0.01,
s/$/ "noisy_seed": 1,
        "noisy_stddev": 0.01, "noisy_seed": 1,
w
1216
q
UNIX>      # Now you'll see the same random charge values generated:
UNIX>
UNIX> ( echo ML tmp.txt ; echo AS 0 0 1 1 1 1; echo RUN 2 ; echo NCH 2) | bin/processor_tool_risp 
Node 2 charge: 0.104594
UNIX> ( echo ML tmp.txt ; echo AS 0 0 1 1 1 1; echo RUN 2 ; echo NCH 2) | bin/processor_tool_risp 
Node 2 charge: 0.104594
UNIX> ( echo ML tmp.txt ; echo AS 0 0 1 1 1 1; echo RUN 2 ; echo NCH 2) | bin/processor_tool_risp 
Node 2 charge: 0.104594
UNIX> 
```

--------------------------------------------------
## Test 13: Weights, inputs_from_weights = false

In `params/risp_weights.json`, there is a parameter file that restricts the synapse
weights to 0.1, 0.5 and 1.0:

```
UNIX> cat params/risp_weights.json
{
  "weights": [ 0.1, 0.5, 1.0 ],
  "inputs_from_weights": false,
  "spike_value_factor": 1,
  "min_threshold": -1,
  "max_threshold": 1,
  "min_potential": -1,
  "max_delay": 5,
  "discrete": false
}
UNIX> 
```

You'll note that I have to specify `inputs_from_weights`, and since it is false, I also
have to specify `spike_value_factor`.  I'm going to create the following network:

![jpg/network_D_weights.jpg](jpg/network_D_weights.jpg)

Here are the commands to create the graph -- pay special attention to how the weights
are specified:

```
UNIX> bin/processor_tool_risp -
- M risp params/risp_weights.json
- EMPTYNET tmp.txt
- Q
UNIX> ../bin/network_tool -
- FJ tmp.txt
- AN 0 1 2 3                     # Add neurons 0-3
- AI 0 1 2                       # Specify that neurons 0-2 are input neurons
- AE 0 3  1 3   2 3              # Add the three synapses
- SEP_ALL Delay 1                # Set all delays to one
- SEP 0 3 Weight 0               # Set the weights: 0 = 0.1, 1 = 0.5 and 2 = 1.0
- SEP 1 3 Weight 1
- SEP 2 3 Weight 2
- SORT
0 1 2 3
- NODES
[ {"id":0,"values":[1.0]},
  {"id":1,"values":[1.0]},
  {"id":2,"values":[1.0]},
  {"id":3,"values":[1.0]} ]
- EDGES
[ {"from":0,"to":3,"values":[0.0,1.0]},    # Again, you'll note that the weight values are indices to the weights array.
  {"from":1,"to":3,"values":[1.0,1.0]},
  {"from":2,"to":3,"values":[2.0,1.0]} ]
- TJ tmp.txt
- Q
UNIX>
```

When we run this, since we specified `inputs_from_weights` to be `false`, and `spike_value_factor`
to be one, we simply specify the spike values in the `apply_spikes()` calls.  Below, I'll make
neurons 0, 1, and 2 spike individually, and we'll look at neuron 3's potential:

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 1                  # Make neuron 0 spike
- RUN 2
- GSR
0 INPUT  : 1
1 INPUT  : 0
2 INPUT  : 0
3 HIDDEN : 0
- NCH
Node 0 charge: 0
Node 1 charge: 0
Node 2 charge: 0
Node 3 charge: 0.1          # This puts 0.1 of charge into neuron 3's potential.

- AS 1 0 1                  # Now make neuron 1 spike.
- RUN 2
- GSR
0 INPUT  : 0
1 INPUT  : 1
2 INPUT  : 0
3 HIDDEN : 0
- NCH
Node 0 charge: 0
Node 1 charge: 0
Node 2 charge: 0
Node 3 charge: 0.6          # This adds 0.5 to neuron 3's potential.

- AS 2 0 1                  # Finally, make neuron 2 spike.
- RUN 2
- GSR
0 INPUT  : 00
1 INPUT  : 00
2 INPUT  : 10
3 HIDDEN : 01               # This adds 1 to neuron 3's potential, so it spikes.
- NCH
Node 0 charge: 0
Node 1 charge: 0
Node 2 charge: 0
Node 3 charge: 0            # And of course its potential is reset to zero.
- Q
UNIX> 
```

--------------------------------------------------
## Test 14: Weights, inputs_from_weights = true

Let's modify the network so that the RISP parameter `inputs_from_weights` is `true`, and
`spike_value_factor` is deleted:

```
UNIX> ed tmp.txt
1174
/input
        "inputs_from_weights": false,
s/false/true/
/spike_value_factor/d
w
1138
q
UNIX> 
```

Now, the input values are restricted to the three synapse weights:

- Calling `apply_spikes()` with a value from 0 to 0.33333 results in a weight of 0.1.  
- Calling `apply_spikes()` with a value from 0.33333 to 0.66667 results in a weight of 0.5.  
- Calling `apply_spikes()` with a value from 0.66667 to 1 results in a weight of 1.0.  

I know that's kind of confusing and convoluted.  We'll probably make some framework changes to make
this better, but it's what you have for now.  Let's illustrate:

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 0                # Calling `apply_spikes()` with a value from 0 to 0.33333 results in a weight of 0.1.
- RUN 1
- NCH 0
Node 0 charge: 0.1        # There it is.

- AS 0 0 0.4              # Calling `apply_spikes()` with a value from 0.33333 to 0.66667 results in a weight of 0.5.  
- RUN 1
- NCH 0
Node 0 charge: 0.6        # Accordingly, 0.5 has been added to neuron 0's potential.
- Q
UNIX> 
```

--------------------------------------------------
## Test 15: Stds

Finally, you can specify a standard deviation for noise for each of the weights in `weights`.  
To demonstrate, I'll set up the following network:

![jpg/network_F_stds.jpg](jpg/network_F_stds.jpg)

We'll use the RISP parameter file in `params/risp_stds.json`

```
UNIX> cat params/risp_stds.json
{
  "weights": [ 0.1, 0.5, 1.0 ],
  "stds": [ 0.01, 0.1, 0.0 ],
  "inputs_from_weights": false,
  "spike_value_factor": 1,
  "min_threshold": -1,
  "max_threshold": 1,
  "min_potential": -1,
  "max_delay": 5,
  "discrete": false
}
UNIX> 
```

We create the network in the exact same manner as above:

```
UNIX> bin/processor_tool_risp -
- M risp params/risp_stds.json
- EMPTYNET tmp.txt
- Q
UNIX> ../bin/network_tool -
- FJ tmp.txt
- AN 0 1 2 3
- AI 0 1 2
- AE 0 3   1 3   2 3
- SEP_ALL Delay 1
- SEP 0 3 Weight 0
- SEP 1 3 Weight 1
- SEP 2 3 Weight 2
- SORT
0 1 2 3
- NODES
[ {"id":0,"values":[1.0]},
  {"id":1,"values":[1.0]},
  {"id":2,"values":[1.0]},
  {"id":3,"values":[1.0]} ]
- EDGES
[ {"from":0,"to":3,"values":[0.0,1.0]},
  {"from":1,"to":3,"values":[1.0,1.0]},
  {"from":2,"to":3,"values":[2.0,1.0]} ]
- TJ tmp.txt
- Q
UNIX> 
```

Now, when we run it and make neurons 0 and 1 spike, you can see the random values
applied to neuron 3.  I spike neuron 2 in the middle to force neuron 3 to spike, so 
you can see those random values better:

```
UNIX> bin/processor_tool_risp -
- ML tmp.txt
- AS 0 0 1                       # Make neuron 0 spike
- RUN 2
- NCH 3
Node 3 charge: 0.0990382         # You can see the random value applied.
- AS 2 0 1                       # This will force neuron 3 to spike.
- AS 1 1 1                       # And neuron 1 will spike one timestep later
- RUN 3
- NCH 3
Node 3 charge: 0.477438          # Here's the value from neuron 2.
- AS 2 0 1
- AS 1 1 1
- RUN 3
- NCH 3
Node 3 charge: 0.411362          # When we repeat the process, we get a different value.
- AS 2 0 1
- AS 0 1 1
- RUN 3
- NCH 3
Node 3 charge: 0.10076           # Ditto from neuron 0.
- Q
UNIX> 
```

---------------------
# Questions, Requests

If you have any questions about RISP or requests for this README, please let me know:
*jplank@utk.edu*.

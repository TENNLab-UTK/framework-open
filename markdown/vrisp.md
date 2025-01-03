# VRISP - Vectorized Reduced Instruction Spiking Processor

Point of contact: Jackson Mowry

-----

[TOC]

-----

# Introduction

VRISP is a specialized variant of the original RISP processor, focused on optimizations to support large densely connected networks, with high levels of activity, and being run on low-power hardware. It differs from RISP on the following features.

  1. Weights, threshold and activations potentials **must** be set to integer values.
  2. A threshold value of `0` means that the neuron will fire every time step (unless it 
     is allowed to hold a charge less than `0`), even if there are no incoming spikes to the neuron.
  3. Emulation of the RAVENs neuroprocessor is not supported.
  4. Random noise addition is not supported.
  
For the above reasons, most networks trained for RISP will work exactly the same on VRISP as long as the following RISP parameters are kept the same.

```javascript
{
  "min_weight": XX,
  "max_weight": XX,
  "min_threshold": 1,
  "max_threshold": XX,
  "min_potential": XX,
  "max_delay": XX,
  "discrete": true
}

```

VRISP additionally adds the concept of `"tracked_timesteps"`, which should be set to `"max_delay" + 1`, although it can be set higher at the cost of greater memory usage. 
 
# Implementations

This repository contains the implementation of the VRISP simulator.

-------------------------------------------------------------------------------

# Default VRISP Parameter Settings

There are only two main types of VRISP processors:
1. VRISP with positive and negative weights: `VRISP-n`
2. VRISP with only positive weights: `VRISP_n+`

You can find common configurations in the `params` directory.

```console
UNIX> ls params/*vrisp* -1
params/vrisp_127.json
params/vrisp_15_plus.json
params/vrisp_1.json
params/vrisp_1_plus.json
params/vrisp_64.json
params/vrisp_7.json
UNIX>
```

As with RISP, all of these configurations have `"leak_mode"` set to `"none"`, meaning you will need to change this setting to either `"all"` or `"configurable"` if your network requires the setting.

`VRISP-n` means that:

- Maximum synapse weight is *n*.
- Minimum synapse weight is *-n*.
- Maxiumum neuron threshold is *n*.
- Minimum neuron threshold is *0*.
- Minimum neuron potential is *-n*.
- Maximum synapse delay is *max(n,15)*.
- Tracked timesteps is *max_delay + 1*

`VRISP-n+1` means that:

- Maximum synapse weight is *n*.
- Minimum synapse weight is *1*.
- Maxiumum neuron threshold is *n*.
- Minimum neuron threshold is *1*.
- Minimum neuron potential is *0*.
- Maximum synapse delay is *max(n,15)*.
- Tracked timesteps is *max_delay + 1*

<!--  LocalWords:  RISP VRISP neuroprocessor RAVENs javascript 
 -->

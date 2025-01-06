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
  5. All run times are exclusive (same as RISP default `run_time_inclusive=false`).
  6. All thresholds are includive (same as RISP default `threshold_inclusive=true`)
  
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

-------------------------------------------------------------------------------
# Params

You can specify the following parameters:
| Key                | Type    | Default      | Description                                                                                                                                                                               |
|--------------------|---------|--------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| min_weight         | integer | Necessary    | The minimum synapse weight.                                                                                                                                                               |
| max_weight         | integer | Necessary    | The maximum snapse weight.                                                                                                                                                                |
| max_delay          | integer | Necessary    | The maximum synapse delay.                                                                                                                                                                |
| min_threshold      | integer | Necessary    | The minimum neuron threshold.                                                                                                                                                             |
| max_threshold      | integer | Necessary    | The maximum neuron threshold.                                                                                                                                                             |
| min_potential      | integer | Necessary    | At the end of integration, the potential cannot go lower than this value.                                                                                                                 |
| tracked_timesteps  | integer | Necessary    | The total number of discrete timestamps that VRISP tracks. Can generally be set to `max_delay+1`, but may be higher if one intends to apply spikes at a time step later than this allows. |
| leak_mode          | string  | "none"       | Leak: `"all"`, `"none"`, `"configurable"`                                                                                                                                                 |
| spike_value_factor | double  | `max_weight` | Framework applications call `apply_spikes()` with input spike values between 0 and 1. VRISP multiplies these values by this factor.                                                       |

------------------------------------------------------------
# Examples of Use

You can go through each of these examples with `scripts/test_vrisp.sh`.





<!--  LocalWords:  RISP VRISP neuroprocessor RAVENs javascript 
 -->

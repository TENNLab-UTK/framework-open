---------
# src/compose_networks.cpp

James S. Plank

This reads two networks, let's call them *n1* and *n2*, and composes them, printing the
composed network on standard output.  You call it with three arguments on the command
line, and `CONFLATE` or `SYNAPSE` lines on standard input.

The arguments are:

```
bin/compose_networks n1 n2 tag
```

The `tag` string is prepended to the name of the neurons on `n2`.  This can be helpful
when you want to differentiate neurons in the final network.

The lines on standard input must be in one of two forms:

```
CONFLATE n1 n2
```

This says that in the final network, there will be just one neuron that is both *n1*
in the first network and *n2* in the second network.  It takes its `values` from the
second network.

```
SYNAPSE from to v0 v1 ...
```

This says to put a synapse from *from* in the first network to *to* in the second network,
and then to set its values to those specified by *v0*, *v1*, etc.   The order of the
values should be the same as defined by the property pack.

------------------------------------------------------------
## Examples 

We're going to compose some XOR, AND and OR networks.  We'll have three examples.

### Example 1 -- Two independent XOR networks.

Let's recall what an XOR network looks like, from [P2021]:

![../img/img/xor_w_leak.jpg](../img/img/xor_w_leak.jpg)

------------------------------------------------------------
## Bibliography

[P2021]: "Spiking Neuromorphic Networks for Binary Tasks", by James S. Plank, ChaoHui Zheng, Catherine D. Schuman and Christopher Dean, ICONS: International Conference on Neuromorphic Systems, 2021.
[https://neuromorphic.eecs.utk.edu/publications/2021-07-29-spiking-neuromorphic-networks-for-binary-tasks/](https://neuromorphic.eecs.utk.edu/publications/2021-07-29-spiking-neuromorphic-networks-for-binary-tasks/)




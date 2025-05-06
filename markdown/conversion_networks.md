# Neuromorphic Conversion Networks for Optimizing Communication

These are conversion networks from the paper:

[PRG2025] James S. Plank, Charles P. Rizzo, Bryson Gullett, Keegan E. M. Dent and Catherine D. Schuman, "Neuromorphic Conversion Networks for Optimizing Communication", submitted for publication, 2025.

If the paper gets accepted, I'll have real citation information for it.  However, these are scripts
to create and exemplify the conversion networks from that paper.

------------------------------------------------------------
## Count-To-Value

This network is too simple to bother with -- please read the paper.

------------------------------------------------------------
## Value-To-Temporal

This is a network that converts a value into a time.  The network is pictured below:

![img/val_to_temporal.jpg](img/val_to_temporal.jpg)

To use this network, spike in a value <i>v</i> from 0 to <i>max</i> to the top neuron at time 0,
and also spike in a value of 1 to the bottom neuron (S) at time 0.  Then run it for
<i>max+2</i> timesteps.  The top neuron will spike at time <i>c+1</i>, where <i>c = max-v</i>.

To exemplify, please use the script in 
[scripts/val_to_temporal.sh](scripts/val_to_temporal.sh).  Here, we'll use a value of 10:

```
UNIX> sh scripts/val_to_temporal.sh 
usage: sh scripts/val_to_temporal.sh max_value framework-open-directory
UNIX> sh scripts/val_to_temporal.sh 10 .
The network tool commands to create this network are in tmp_network_tool.txt.
The network itself is in tmp_network.txt.
Please run the following processor tool commands to exemplify.
Do RSC instead of RUN to see the neuron charges and the spikes.
----------------------------------------
Example 1: Val = 0
Do the following, and the result should be that neuron 0 fires at time 11 .
Do RSC instead of RUN to see the neuron charges and the spikes.
( echo ML tmp_network.txt 
  echo ASV 0 0 0
  echo ASV 1 0 1
  echo RUN 12 
  echo OT ) | ./bin/processor_tool_risp

Example 2: Val = 10
Do the following, and the result should be that neuron 0 fires at time 1 .
Do RSC instead of RUN to see the neuron charges and the spikes.
( echo ML tmp_network.txt 
  echo ASV 0 0 10
  echo ASV 1 0 1
  echo RUN 12 
  echo OT ) | ./bin/processor_tool_risp

Example 3: Val = 5
Do the following, and the result should be that neuron 0 fires at time 6 .
Do RSC instead of RUN to see the neuron charges and the spikes.
( echo ML tmp_network.txt 
  echo ASV 0 0 5
  echo ASV 1 0 1
  echo RUN 12 
  echo OT ) | ./bin/processor_tool_risp

UNIX> 
```

Let's do the last example to see that neuron 0 does indeed fire at time 6:

```
UNIX> ( echo ML tmp_network.txt
    echo ASV 0 0 5
    echo ASV 1 0 1
    echo RUN 12
    echo OT ) | ./bin/processor_tool_risp
node 0(Val_v) spike times: 6.0
UNIX> 
```

Let's do `RSC 12` isntead of `RUN 12` to see the two neurons as they perform the conversion:

```
UNIX> ( echo ML tmp_network.txt
        echo ASV 0 0 5
        echo ASV 1 0 1
        echo RSC 12 ) | ./bin/processor_tool_risp
Time 0(Val_v)     1(S) | 0(Val_v)     1(S)
   0        -        * |        5        0
   1        -        * |        6        0
   2        -        * |        7        0
   3        -        * |        8        0
   4        -        * |        9        0
   5        -        * |       10        0
   6        *        * |        0        0
   7        -        - |        0        0
   8        -        - |        0        0
   9        -        - |        0        0
  10        -        - |        0        0
  11        -        - |        0        0
UNIX> 
```

This is testing script 51, btw:

```
UNIX> sh scripts/test_risp.sh 51 no
Passed Test 51 - val_to_temporal encoding network with a maximum value of 10 (from [PRG25]).
UNIX> 
```

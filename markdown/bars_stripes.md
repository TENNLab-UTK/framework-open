# The Bars and Stripes problem, and a RISP solution

This problem comes from the paper "An FPGA-Based Neuromorphic Processor with All-to-All
Connectivity" [Maheshwari23].  You are given an input of *r x c* black or white pixels,
and you wnat to determine whether the input has at least one vertical bar or horizontal
stripe.  Here are four examples:

![img/bs_example.jpg](../img/bs_example.jpg)

In [Maheshwari23], they give a simple spiking neural network that performs the calculation.
There are *r x c* input neurons, one for each pixel, and you spike in all of the black pixels
at time zero.  There are two output neurons -- one for bars (vertical lines) and one for
stripes (horizontal lines).  You run the network for three timesteps, and at the last timestep, 
each output neuron spikes if there is a bar or stripe.  

## Script to create the network

There is a shell script, `scripts/bars_stripes.sh`, that creates a RISP network for the
bars and stripes problem.  You give it the number of rows and columns as command line arguments,
and it outputs the RISP network (which is a RISP-1+ network modified so that thresholds
are higher, and leak is on) that solves the problem.

Here's an example of creating a 2x3 network:

```
UNIX> sh scripts/bars_stripes.sh
usage: sh scripts/bars_stripes.sh rows cols
UNIX> sh scripts/bars_stripes.sh 2 3 > tmp_network.txt
UNIX> ( echo FJ tmp_network.txt ; echo INFO ) | bin/network_tool
Nodes:         13
Edges:         17
Inputs:         6
Outputs:        2

Input nodes:  7(I[0][0]) 8(I[0][1]) 9(I[0][2]) 10(I[1][0]) 11(I[1][1]) 12(I[1][2]) 
Hidden nodes: 6(H-B2) 2(H-S0) 3(H-S1) 4(H-B0) 5(H-B1) 
Output nodes: 0(O-Bar) 1(O-Stripe) 
UNIX> 
```

As you can see, the input neurons start at neuron 7, and outputs are neurons 0 and 1 (for bar
and stripe).  Let's see it in action.  Suppose the input contains a bar in the last column:

```
001
001
```

That corresponds to spiking neurons 9 and 12.  Let's do that and then run the network for three
timesteps.  If all is working, neuron 0 will spike at timestep 2:

```
UNIX> ( echo ML tmp_network.txt
        echo AS 9 0 1
        echo AS 12 0 1
        echo RUN 3
        echo OT ) | bin/processor_tool_risp
node 0(O-Bar) spike times: 2.0
node 1(O-Stripe) spike times:
UNIX> 
```

Nice -- let's do a stripe too -- we'll spike neurons 10, 11 and 12:

```
UNIX> ( echo ML tmp_network.txt
>       echo AS 10 0 1    11 0 1    12 0 1
>       echo RUN 3
>       echo OT ) | bin/processor_tool_risp
node 0(O-Bar) spike times:
node 1(O-Stripe) spike times: 2.0
UNIX> 
```

----------
## A script to test bigger networks

The script in `scripts/bars_inputs.sh` takes four parameters -- a bars-and-stripes network,
the number of
rows and columns, and the starting input neuron.  It reads a rectangle of *rows x cols*
pixels, and then it creates the `processor_tool` commands to run the network on the input.

I have the four examples from above in the files `txt/7x7-bar.txt`, `txt/7x7-stripe.txt`,
`txt/7x7-both.txt` and `txt/7x7-none.txt`.  Let's run them:

```
UNIX> sh !$ sh scripts/bars_input.sh
usage: sh scripts/bars_input.sh network rows cols starting_neuron < pixels
UNIX> sh scripts/bars_stripes.sh 7 7 > tmp_network.txt                          # Create the network
UNIX> ( echo FJ tmp_network.txt ; echo INFO ) | bin/network_tool
Nodes:         65
Edges:        112
Inputs:        49
Outputs:        2
                                   # We can see from the next line that neuron 16 is the first input.
Input nodes:  16(I[0][0]) 17(I[0][1]) 18(I[0][2]) 19(I[0][3]) 20(I[0][4]) ...
Hidden nodes: 6(H-S4) 11(H-B2) 7(H-S5) 15(H-B6) 3(H-S1) 14(H-B5) 2(H-S0) ...
Output nodes: 0(O-Bar) 1(O-Stripe) 
UNIX> sh scripts/bars_input.sh tmp_network.txt 7 7 16 < txt/7x7-bar.txt     # The script produces processor_tool commands
ML tmp_network.txt
AS 21 0 1
AS 25 0 1
AS 28 0 1
AS 35 0 1
AS 38 0 1
AS 42 0 1
AS 49 0 1
AS 55 0 1
AS 56 0 1
AS 58 0 1
AS 63 0 1
RUN 3
OT
UNIX>                 # We run it on the four inputs to see that it works properly.
UNIX> sh scripts/bars_input.sh tmp_network.txt 7 7 16 < txt/7x7-bar.txt | bin/processor_tool_risp 
node 0(O-Bar) spike times: 2.0
node 1(O-Stripe) spike times:
UNIX> sh scripts/bars_input.sh tmp_network.txt 7 7 16 < txt/7x7-stripe.txt | bin/processor_tool_risp 
node 0(O-Bar) spike times:
node 1(O-Stripe) spike times: 2.0
UNIX> sh scripts/bars_input.sh tmp_network.txt 7 7 16 < txt/7x7-both.txt | bin/processor_tool_risp 
node 0(O-Bar) spike times: 2.0
node 1(O-Stripe) spike times: 2.0
UNIX> sh scripts/bars_input.sh tmp_network.txt 7 7 16 < txt/7x7-none.txt | bin/processor_tool_risp 
node 0(O-Bar) spike times:
node 1(O-Stripe) spike times:
UNIX> 
```

----------
## Reference

[Maheshwari23] D. Maheshwari, A. Young, P. Date, S. Kulkarni, B.  Witherspoon and N. R. Miniskar, "An FPGA-Based Neuromorphic Processor with All-to-All Connectivity," IEEE International Conference on Rebooting Computing (ICRC), 10.1109/ICRC60800.2023.10386808, 2023, pp. 1-5.


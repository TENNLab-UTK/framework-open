# More Examples of the Cart-Pole Application

These come from the paper, 
"The Cart-Pole Application as a Benchmark for Neuromorphic Computing", by Plank,
Rizzo, White and Schuman.  I'll put a link to the paper when I have it, and when
I refer to it in this markdown file, I'll refer to it as [PRWS2024].

To walk through these examples, or to have instructions on how to use them on
Gymnasium, please see [the main cart-pole example](cartpole_example.md).
Examples 1, 2, 7 and 8 work with Gymnasium pretty much out of the box.  The other
four examples have an addition action, "Do-Nothing," so they require some glue to
work with Gymnasium.

Regardless, for each example, we provide `processor_tool` commands so that
you can see it in action without Gynmasium or any cart-pole application for that
matter.  We also include videos so that you can see it working, along with the
`processor_tool` commands.

---------
## Example 1 - Figure 7(a).  A RISP-1+ network for the *Easy* benchmark

This is Figure 7(a) from [PRWS2024].  It is a RISP-1+ network, so all synapse
weights are one, and all neuron thresholds are one.  The delays are shown on
the figure:

![PRWS-2024-7a.png](images/PRWS-2024-7a.png)

You can create this network with the testing script, which puts it into the
file `tmp_network.txt`:

```
UNIX> sh scripts/test_risp.sh 42 yes
Passed Test 42 - Cart-pole example from [PRWS2024], Figure 7a.
UNIX> ( echo FJ tmp_network.txt ; echo INFO ) | bin/network_tool
Nodes:         10
Edges:          7
Inputs:         8
Outputs:        2

Input nodes:  0 1 2 3 4 5 6 7 
Hidden nodes: 
Output nodes: 8 9 
UNIX> 
```

In the following video, we show our cart-pole application running with this
network as the agent.  Values are encoded and decoded as explained in
[the first cart-pole example](cartpole_example.md).  In the video, you can 
also see on the terminal screen: the observations, actions, input spikes,
spike raster output, and processor tool commands for each timestep.  It
is slowed by 50%, but you can pause and step through the video to see
individual values.

[Example-7a-video](https://youtu.be/0jozLerSaxI)

When you run test script 42, the file
`tmp_pt_input.txt` gives you the processor tool commands that are being
used in the video.  Along with them are the outputs from the teriminal in 
the video, included as comments.  So, for example, take a look at 
the first 27 lines of `tmp_pt_input.txt`:

```
UNIX> head -n 27 tmp_pt_input.txt
ML tmp_network.txt
# Step 0000. Observations: -1.17261 0.201336 -0.0686158 0.40251
# Step 0000. I-Spike-C: 4 0 0 1 3 0 0 2
AS 0 0 1
AS 0 3 1
AS 0 6 1
AS 0 9 1
AS 3 0 1
AS 4 0 1
AS 4 3 1
AS 4 6 1
AS 7 0 1
AS 7 3 1
RUN 24
GSR
# Step 0000. R: 0      1001001001000000000000
# Step 0000. R: 1      0000001001000000000000
# Step 0000. R: 2      0000000000000000000000
# Step 0000. R: 3      1000000000000000000000
# Step 0000. R: 4      1001001000000000000000
# Step 0000. R: 5      0000001001000000000000
# Step 0000. R: 6      0000000000000000000000
# Step 0000. R: 7      1001000000000000000000
# Step 0000. R: 8      0000001001001000000000
# Step 0000. R: 9      0000000000000010011001
# Step 0000. O-Spike-C: 3 4
# Step 0000. Action: 1
UNIX> 
```

If we run these lines in the processor_tool, the spike raster matches:

```
UNIX> head -n 27 tmp_pt_input.txt | bin/processor_tool_risp 
0 INPUT  : 1001001001000000000000
1 INPUT  : 0000001001000000000000
2 INPUT  : 0000000000000000000000
3 INPUT  : 1000000000000000000000
4 INPUT  : 1001001000000000000000
5 INPUT  : 0000001001000000000000
6 INPUT  : 0000000000000000000000
7 INPUT  : 1001000000000000000000
8 OUTPUT : 0000001001001000000000
9 OUTPUT : 0000000000000010011001
UNIX> 
```

You can map this to the video if you want.  For example, here's a screenshot
of the video, paused at the 3.48 second mark:

![PRWS-2024-7a-SS.jpg](images/PRWS-2024-7a-SS.jpg)

Now, let's take a look at the output of the `processor_tool` for those
steps.  Each time-raster has 10 neurons, so if we want timestep 173, 
we'll want lines 1731 to 1740 of the output:

```
UNIX> bin/processor_tool_risp < tmp_pt_input.txt | cat -n | sed -n 1731,1740p
  1731	0 INPUT  : 000000000000
  1732	1 INPUT  : 100000000000
  1733	2 INPUT  : 000000000000
  1734	3 INPUT  : 100100000000
  1735	4 INPUT  : 100101000000
  1736	5 INPUT  : 000000000000
  1737	6 INPUT  : 100000000000
  1738	7 INPUT  : 000000000000
  1739	8 OUTPUT : 000000100101
  1740	9 OUTPUT : 000000001000
UNIX> 
```

See how that matches the spike raster for step 173 in the screenshot?  Cool, no?

Remember, you can also use this network in Gymnasium -- you'll need to
read through 
[the first cart-pole example](cartpole_example.md).






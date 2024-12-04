# Example of a neuromorphic implementation of a Cart Pole agent #

Cart Pole (also known as the inverted pendulum problem) is arguably **the** quintessential control theory problem. It is a problem that has been solved by numerous algorithms in the literature over the past few decades. It features a pole that must be balanced upright on a cart that can only move along the x axis where failure occurs if the pole's angle *theta* falls outside of some user defined range. The Cart Pole environment we will use in this example comes from Gymnasium and can be found here: https://gymnasium.farama.org/environments/classic_control/cart_pole/.

We use the same RISP-1+ network that is discussed in the paper "The RISP Neuroprocessor - Open Source Support for Embedded Neuromorphic Computing" (params file in [params/risp_1_plus.txt](params/risp_1_plus.txt). The network was trained with the genetic algorithm EONS on an internal application that we call Polebalance. The Polebalance application is nearly identical to Cart Pole, and we show how to run this network on the Cart Pole problem later.

We can create the network by running testing script 40 or 41.  The network will be in
`tmp_network.txt`.
We can use the `network_tool` to show us basic information about the network:

```
UNIX> sh scripts/test_risp.sh 40 yes
Passed Test 40 - Cart-pole example on the observations: [x=0.852, dx=-0.007, t=0.018, dt=-0.659].
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

The network is very small: it is only 10 neurons and 7 synapses. It was created using EONS, and it works in the following way:

## Converting inputs to spikes ##

- There are four input values for each observation of Polebalance / Cart Pole. The table below shows the ranges of each value for each application. The reason Polebalance has different values for Cart Position and Pole Angle is because, in the Gymnasium defined Cart Pole application, the episode terminates if the cart is outside of the [-2.4, 2.4] range or if the pole's angle is outside of the [-12º, 12º] range. We therefore saw no reason to encode values outside of those ranges. Additionally, we found the range of [-2.0, 2.0] to be sufficient for both Cart Velocity and Pole Angular Velocity

    | **CartPole** |                       |                     |                   | **Polebalance** |                   |                 |
    |:------------:|:---------------------:|:-------------------:|-------------------|-----------------|-------------------|-----------------|
    | **Num**      | **Input**             | **Min**             | **Max**           |                 | **Min**           | **Max**         |
    | 0            | Cart Position         | -4.8                | 4.8               |                 | -2.4              | 2.4             |
    | 1            | Cart Velocity         | -Inf                | Inf               |                 | -2.0              | 2.0             |
    | 2            | Pole Angle            | ~ -0.418 rad (-24º) | ~ 0.418 rad (24º) |                 | -0.209 rad (-12º) | 0.209 rad (12º) |
    | 3            | Pole Angular Velocity | -Inf                | Inf               |                 | -2.0              | 2.0             |

- The inputs are encoded using a "flip-flop" encoder where there are two neurons per input and up to 8 spikes per neuron. Negative values are spiked into one neuron and positive values are spiked into the other neuron for each input. The more negative the value, the greater the amount of spikes (up to a maximum of 8 spikes) that is applied to the "negative" neuron. The greater the positive value, the greater the amount of spikes that is applied to the "positive" neuron (again, up to a maximum of 8 spikes).
- We apply spikes every third timestep.

## Running the neural network and converting the output spikes to a value ##

There is only one output for the Cart Pole and Polebalance applications: move the cart along the x axis. This output is split into two neurons, one for "push the cart left" and the other for "push the cart right." The spiking neural network runs on an observation for 24 timesteps, and then the counts of the two output neurons are compared. Whichever output neuron fires the most determines whether the cart will be pushed to the right or to the left. If there is a tie between the output neurons' fires, the action of the neuron with the lowest id will be chosen -- in this case, that is "move left". This is also popularly known as the "Winner-takes-all" (WTA) decoder.

## Two examples

These examples are in testing scripts 40 and 41.  Let's run test 40 again to see how it
works:

```
UNIX> sh scripts/test_risp.sh 40 yes
Passed Test 40 - Cart-pole example on the observations: [x=0.852, dx=-0.007, t=0.018, dt=-0.659].
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

We convert the inputs to spikes as follows:

- 0.852/2.4*8 = 2.84, which maps to three spikes on neuron 1 (neuron 0 is for negative 
  values of *x*).
- 0.007/2*8 = 0.027, which maps to one spike on neuron 2 (neuron 3 is for positive values of *dx*).
- 0.018/0.209*8 = 0.69, which maps to one spike on neuron 5 (neuron 4 is for negative values of *theta*).
- 0.659/2*8 = 2.639, which maps to three spikes on neuron 6 (neuron 7 is for positive values of *d-theta*).

Accordingly, the processor_tool commands for the example are in `tmp_pt_input.txt`:

```
UNIX> cat tmp_pt_input.txt 
ML tmp_network.txt
AS 1 0 1
AS 1 3 1
AS 1 6 1
AS 2 0 1
AS 5 0 1
AS 6 0 1
AS 6 3 1
AS 6 6 1
RUN 24
GSR
OC
UNIX> 
```

When we run it, we see 4 spikes on each of neurons 8 and 9.  The tie goes to "push left":

```
UNIX> bin/processor_tool_risp < tmp_pt_input.txt
0 INPUT  : 000000000000000000
1 INPUT  : 100100100000000000
2 INPUT  : 100000000000000000
3 INPUT  : 000000000000000000
4 INPUT  : 010001001001000000
5 INPUT  : 100000000000000000
6 INPUT  : 100100100000000000
7 INPUT  : 000000000000000000
8 OUTPUT : 000000010001001001
9 OUTPUT : 000000001001101000
node 8 spike counts: 4
node 9 spike counts: 4
UNIX> 
```

You can run through test 41 similarly.

## Making the Polebalance-trained network play Cart Pole ##

Fortunately, using the Polebalance-trained network to play Cart Pole is pretty straightforward. We use `processor_tool_risp` to run `tmp_network.txt` alongside an instance of Cart Pole. In this way, we can use a Python program to launch the Cart Pole instance and pass spikes to the `processor_tool_risp` instance in real time using pipes. We can then poll the output of the network from `processor_tool_risp`, perform the WTA decoding, and communicate the appropriate action back to the Cart Pole instance. We show some Python code to accomplish this below:

```python
import re
import gym
import subprocess
import math

# Make and initialize CartPole-v1 gym instance
env = gym.make('CartPole-v1')
env.reset()

# Launch processor_tool_risp subprocess and load into it the tmp_network.txt network
Prog = subprocess.Popen(['/.../framework-open/bin/processor_tool_risp'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
Prog.stdin.write("ML tmp_network.txt\n")
Prog.stdin.flush()
Prog.stdin.write("INFO\n")
Prog.stdin.flush()

# Identify the Output neuron ids for "move left" and "move right"
for line in Prog.stdout:
    if line.find("Output") != -1:
        node_left, node_right = re.findall('[0-9]+', line)
        break

# Extract "sim_time" from the network file
with open("tmp_network.txt", "r") as network_file:
    buf = network_file.readlines()
    subbuf = buf[0].split("sim_time")
    sim_time = re.findall('[0-9]+', subbuf[1])[0]

# Define input values' ranges in a list
input_value_range = [[-2.4,2.4],
                     [-2.0,2.0],
                     [-0.209,0.209],
                     [-2.0,2.0]]

# Begin Gym loop and continue running while the episode has not been terminated for up to 15,000 timesteps
timesteps = 0
while env.steps_beyond_terminated == None and timesteps <= 15000:  

    # Scale the cart position and pole angle values from the Cart Pole ranges to Polebalance ranges
    # It is worth noting that the network still successfully solves Cart Pole without doing this
    scaled_cart_pos = ((env.state[0] - -4.8) / (4.8 - -4.8)) * (2.4 - -2.4) + -2.4
    scaled_pole_angle = ((env.state[2] - -0.418) / (0.418 - -0.418)) * (0.209 - -0.209) + -0.209
 

    # Encode the four values into spikes using the same 2-bin, "flip-flop" encoding scheme
    val_index = 0
    obs = ""
    for val in [scaled_cart_pos,env.state[1],scaled_pole_angle,env.state[3]]:
        bin_size = ((input_value_range[val_index][1] - input_value_range[val_index][0]) / 2)
        max_spikes = 8

        # Apply input to bin 0 if the value is negative
        if val <= 0:
            percentage = 1 - (val - input_value_range[val_index][0]) / bin_size
            bin_offset = 0
        # Apply input to bin 1 if the value is positive
        else:
            percentage = val / bin_size
            bin_offset = 1

        # We can only apply a maximum of 8 spikes, so we cap the percentage 1.0
        if percentage > 1:
            percentage = 1

        num_spikes = int(math.ceil(percentage * max_spikes))

        for j in range(0,num_spikes):
            obs += "AS %d %d %d\n" % (val_index * 2 + bin_offset,j * 3,1)

        val_index += 1

    # Print the "obs" variable if you want to see the list of Apply_Spike (AS) calls passed to `processor_tool_risp`
    Prog.stdin.write(obs)

    # Run the network for "sim_time" timesteps and poll output neuron fires
    Prog.stdin.write("RUN " + sim_time + "\n")
    Prog.stdin.write("OC\n")
    Prog.stdin.flush()

    # Determine the number of fires the "move left" and "move right" neurons each have
    for line in Prog.stdout:
        if line.find("node " + node_left) != -1:
            spike_left = int(line[len(line)-2])
        if line.find("node " + node_right) != -1:
            spike_right = int(line[len(line)-2])
            break

    # Perform the WTA decoding. If "move left" neuron fires more, perform the "move left" action. Same with "move right".
    move = max(spike_left, spike_right)
    if spike_left == move:
        array = env.step(0)
    else:
        array = env.step(1)

    timesteps += 1

# Print final timesteps survived
print("Fitness " + str(timesteps-1))
```

## Visualization of tmp_network.txt playing Cart Pole ##

A 30 second clip of the `tmp_network.txt` file playing Cart Pole as shown above can be found [here](https://youtu.be/cGYg4RSb5Pw). 

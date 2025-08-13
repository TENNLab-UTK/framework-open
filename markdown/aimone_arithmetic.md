# Arithmetic Networks from Aimone, Hill, Severa and Vineyard, 2021.

James S. Plank

In this markdown file, we go over scripts to implement arithmetic using
the networks defined in the paper [AHSV2021]: "Spiking Neural Streaming Binary Arithmetic",
by James B. Aimone, Aaron J. Hill, William M. Severa, & Craig M. Vineyard, 
*IEEE International Conference on Rebooting Computing*, 2021.
You can get the paper from [https://www.computer.org/csdl/proceedings-article/icrc/2021/233200a079/1CbZFjqAqju](https://www.computer.org/csdl/proceedings-article/icrc/2021/233200a079/1CbZFjqAqju), (or on arXiv).

----------------------------------------
# Addition of two numbers.

The following network, from [AHSV2021], adds two numbers when they are streamed in little-endian.
(As always, if unlabeled, neuron thresholds are one, synapse weights are 1, and synapse delays or 1.
Red synapses have weights of -1.).

![../img/Aimone_Adder.jpg](../img/Aimone_Adder.jpg)

Now, let's construct a network and test it out.  The shell script
[scripts/aimone_adder.sh](../scripts/aimone_adder.sh) does all of the work for you:

```
UNIX> sh scripts/aimone_adder.sh 
usage: sh scripts/aimone_adder.sh v1 v2 os_framework
UNIX> sh scripts/aimone_adder.sh 15 85 .
Inputs in little endian: 1111000 1010101
w: 7
Time  0(A)  1(B) 2(S1) 3(S2) 4(S3)  5(O) |  0(A)  1(B) 2(S1) 3(S2) 4(S3)  5(O)
   0     *     *     -     -     -     - |     0     0     0     0     0     0
   1     *     -     *     *     -     - |     0     0     0     0     0     0
   2     *     *     *     *     -     - |     0     0     0     0     0     0
   3     *     -     *     *     *     - |     0     0     0     0     0     0
   4     -     *     *     *     -     * |     0     0     0     0     0     0
   5     -     -     *     *     -     - |     0     0     0     0     0     0
   6     -     *     *     -     -     - |     0     0     0     0     0     0
   7     -     -     *     -     -     * |     0     0     0     0     0     0
   8     -     -     -     -     -     * |     0     0     0     0     0     0
   9     -     -     -     -     -     - |     0     0     0     0     0     0
Sum in Little Endian: 00100110
Sum in Decimal: 100
UNIX> 
```

In the shell script, I convert the two values to binary, little endian.  So 15 becomes
1111000 and 85 becomes 1010101.  You can see above how the network computes the sum,
whose little endian starts at timestep 2: 00100110.  that equals 100 in decimal,
which of course is correct.

In this script and the others in this markdown writeup, when you're done running
it, it creates the following files:

- `tmp_network.txt` - The RISP network.
- `tmp_network_tool.txt` - The `network_tool` commands to create the network.
- `tmp_pt_input.txt` - The input to `processor_tool_risp` that gives the output above.
- `tmp_pt_output.txt` - The output of `processor_tool_risp` on the input.
- `tmp_empty.txt` - The "empty" RISP network that the `network_tool` uses as a starting 
   network.
- `tmp_risp.txt` - The RISP JSON to create the empty network.  

You'll note that since this network requires leak, the RISP settings make sure that the
neurons leak their charge at every timestep.

```
UNIX> cat tmp_pt_input.txt
ML tmp_network.txt
ASR 0 1111000
ASR 1 1010101
RSC 10
UNIX> bin/processor_tool_risp < tmp_pt_input.txt
Time  0(A)  1(B) 2(S1) 3(S2) 4(S3)  5(O) |  0(A)  1(B) 2(S1) 3(S2) 4(S3)  5(O)
   0     *     *     -     -     -     - |     0     0     0     0     0     0
   1     *     -     *     *     -     - |     0     0     0     0     0     0
   2     *     *     *     *     -     - |     0     0     0     0     0     0
   3     *     -     *     *     *     - |     0     0     0     0     0     0
   4     -     *     *     *     -     * |     0     0     0     0     0     0
   5     -     -     *     *     -     - |     0     0     0     0     0     0
   6     -     *     *     -     -     - |     0     0     0     0     0     0
   7     -     -     *     -     -     * |     0     0     0     0     0     0
   8     -     -     -     -     -     * |     0     0     0     0     0     0
   9     -     -     -     -     -     - |     0     0     0     0     0     0
UNIX> ( echo FJ tmp_network.txt ; echo INFO ) | bin/network_tool
Nodes:          6
Edges:         12
Inputs:         2
Outputs:        1

Input nodes:  0(A) 1(B) 
Hidden nodes: 4(S3) 2(S1) 3(S2) 
Output nodes: 5(O) 
UNIX> 
```
----------------------------------------
# Inversion



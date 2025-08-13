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

:q


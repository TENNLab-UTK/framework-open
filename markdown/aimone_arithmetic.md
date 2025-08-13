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

![../img/Aimone_Adder.jpg](../img/Aimone_Adder.jpg)


# This creates a network that adds two numbers that are fed in a binary spike train,
# in little endian.  It works with positive and negative numbers, where the negative
# numbers are converted with two's complement.
#
# James S. Plank.  September, 2025.
#
# This network comes from the paper "Spiking Neural Streaming Binary Arithmetic",
# by James B. Aimone, Aaron J. Hill, William M. Severa, & Craig M. Vineyard, IEEE
# International Conference on Rebooting Computing, 2021. You can get the paper
# from https://www.computer.org/csdl/proceedings-article/icrc/2021/233200a079/1CbZFjqAqju,
# (or on arXiv).
#
# The network is printed on standard output, and information for running is
# put into tmp_info.txt.  

if [ $# -ne 2 ]; then
  echo 'usage: sh scripts/adder_network.sh w os_framework' >&2
  exit 1
fi

w=$1
fro=$2

# Compile necesssary programs in the open-source framework

for i in network_tool processor_tool_risp ; do
  if [ ! -x $fro/bin/$i ]; then ( cd $fro ; make bin/$i ) fi
done

# Make an empty network with the proper RISP parameters.

cat $fro/params/risp_127.txt | sed '/leak_mode/s/none/all/' > tmp_risp.txt

( echo M risp tmp_risp.txt
  echo EMPTYNET tmp_emptynet.txt ) | $fro/bin/processor_tool_risp

# Create the network and put print it on standard output.

( echo FJ tmp_emptynet.txt
  echo AN 0 1 2 3 4 5 6
  echo AI 0 1 2
  echo AO 6
  echo SETNAME 0 V0
  echo SETNAME 1 V1
  echo SETNAME 2 S
  echo SETNAME 3 S1
  echo SETNAME 4 S2
  echo SETNAME 5 S3
  echo SETNAME 6 SUM
  echo SNP_ALL Threshold 1
  echo SNP 4 Threshold 2
  echo SNP 5 Threshold 3

  echo AE 0 3   0 4   0 5
  echo AE 1 3   1 4   1 5
  echo AE 2 6
  echo AE 3 6
  echo AE 4 3   4 4   4 5   4 6
  echo AE 5 6

  echo SEP_ALL Weight 1
  echo SEP_ALL Delay 1
  echo SEP 4 6 Weight -1
  echo SEP 2 6 Weight -1
  echo SEP 2 6 Delay $(($w+2))
  
  echo SORT Q
  echo TJ
  ) > tmp_network_tool.txt

$fro/bin/network_tool < tmp_network_tool.txt

# Info is the form:
#
# INPUT  node_name node_number format timesteps starting_timestep
# OUTPUT node_name node_number format timesteps starting_timestep
# RUN timesteps

( echo INPUT  V0  0 TC_LE $w   0 
  echo INPUT  V1  1 TC_LE $w   0 
  echo INPUT  S   2 Spike 1    0 
  echo OUTPUT SUM 6 TC_LE $w 2 
  echo RUN $(($w+3))
) > tmp_info.txt

rm -f tmp_risp.txt
rm -f tmp_emptynet.txt
rm -f tmp_network_tool.txt

# This creates a network that takes a w-bit number, represented as spikes, and flips
# the bits.  We use this as a starting point for the twos_complement network, so this
# isn't the most useful network, but there you have it.
#
# James S. Plank.  September, 2025.
#
# This network is inspired by the paper "Spiking Neural Streaming Binary Arithmetic",
# by James B. Aimone, Aaron J. Hill, William M. Severa, & Craig M. Vineyard, IEEE
# International Conference on Rebooting Computing, 2021. You can get the paper
# from https://www.computer.org/csdl/proceedings-article/icrc/2021/233200a079/1CbZFjqAqju,
# (or on arXiv).  However, their network leverages a feature of TrueNorth that has neurons
# "leak" positive values.  Our neurons don't work that way, so we instead set up a bias
# neuron.
# 
#
# The network is printed on standard output, and information for running is
# put into tmp_info.txt.

if [ $# -ne 2 ]; then
  echo 'usage: sh scripts/inversion_network.sh w os_framework' >&2
  exit 1
fi

w=$1
fro=$2

# Make sure the network_tool and processor_tool are compiled.

for i in network_tool processor_tool_risp ; do
  if [ ! -x $fro/bin/$i ]; then
    ( cd $fro ; make bin/$i )
  fi
done

# Make an empty network with the proper RISP parameters.
# We'll go ahead and turn leak on, since the other networks use that.

cat $fro/params/risp_127.txt | sed '/leak_mode/s/none/all/' > tmp_risp.txt

( echo M risp tmp_risp.txt
  echo EMPTYNET tmp_emptynet.txt ) | $fro/bin/processor_tool_risp

# Now create the network.

( echo FJ tmp_emptynet.txt
  echo AN 0 1 2 3 4
  echo AI 0 1
  echo AO 4
  echo SETNAME 0 A
  echo SETNAME 1 S
  echo SETNAME 2 C
  echo SETNAME 3 Bias
  echo SETNAME 4 Inv
  echo SNP_ALL Threshold 1

  echo AE 0 4 
  echo AE 1 4   1 3  1 2
  echo AE 3 3   3 4
  echo AE 2 3

  echo SEP_ALL Weight 1
  echo SEP_ALL Delay 1
  echo SEP 0 4 Weight -1
  echo SEP 1 2 Delay $(($w-1))
  echo SEP 2 3 Weight -1
  
  echo SORT Q
  echo TJ
  
  ) > tmp_network_tool.txt

$fro/bin/network_tool < tmp_network_tool.txt

# Info is the form:
#
# INPUT  node_name node_number format timesteps starting_timestep
# OUTPUT node_name node_number format timesteps starting_timestep
# RUN timesteps

( echo INPUT  V0  0 Bits $w   0
  echo INPUT  S   1 Spike 1    0
  echo OUTPUT Inv 4 Bits $w 1
  echo RUN $(($w+1))
) > tmp_info.txt

rm -f tmp_risp.txt
rm -f tmp_emptynet.txt
rm -f tmp_network_tool.txt


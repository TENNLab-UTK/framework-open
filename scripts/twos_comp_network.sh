# This creates a network that takes a w-bit number, represented as spikes in 
# little-endian, twos-complement, and outputs the two's-complement of the number.
#
# James S. Plank.  September, 2025.
#
# This network was created by composing the inversion network of scripts/inversion_network.sh
# and the adder of scripts/adder_network.sh, and then optimizing.
#
# The network is printed on standard output, and information for running is
# put into tmp_info.txt.

if [ $# -ne 2 ]; then
  echo 'usage: sh scripts/twos_complement_w.sh $w os_framework' >&2
  exit 1
fi

w=$1
fro=$2

# Make an empty network with the proper RISP parameters.
# By default, leak is on.

cat $fro/params/risp_127.txt | sed '/leak_mode/s/none/all/' > tmp_risp.txt

( echo M risp tmp_risp.txt
  echo EMPTYNET tmp_emptynet.txt ) | $fro/bin/processor_tool_risp

# Now create the network and print on stdout

( echo FJ tmp_emptynet.txt
  echo AN 0 1 2 3 4 5 6
  echo AI 0 1
  echo AO 6
  echo SETNAME 0 A
  echo SETNAME 1 S
  echo SETNAME 2 C
  echo SETNAME 3 Bias
  echo SETNAME 4 S1
  echo SETNAME 5 S2
  echo SETNAME 6 O

  echo SNP_ALL Threshold 1
  echo SNP 5 Threshold 2

  echo AE 0 4   0 5
  echo AE 1 2   1 3   1 4   1 5
  echo AE 2 3
  echo AE 3 3   3 4   3 5
  echo AE 4 6
  echo AE 5 4   5 5   5 6

  echo SEP_ALL Weight 1
  echo SEP_ALL Delay 1
  echo SEP 0 4    0 5   Weight -1
  echo SEP 1 4    1 5   Weight 2
  echo SEP 1 2 Delay $(($w-1))
  echo SEP 2 3 Weight -1
  echo SEP 5 6 Weight -1

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
  echo INPUT  S   1 Spike 1    0
  echo OUTPUT O   6 TC_LE $w 2
  echo RUN $(($w+2))
) > tmp_info.txt

rm -f tmp_risp.txt
rm -f tmp_emptynet.txt
rm -f tmp_network_tool.txt


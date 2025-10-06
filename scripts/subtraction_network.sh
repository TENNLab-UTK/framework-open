# This creates a network that subtracts two numbers that are fed in a binary spike train,
# in little endian.  It works with positive and negative numbers, where the negative
# numbers are converted with two's complement.
#
# w is used to isolate the output to w bits.
#
# James S. Plank.  September, 2025.
#
# This network was created by composing a two's complement inverted with an adder,
# and then coalescing nodes as appropriate to make the network more efficient.
# You can remove the synapse from C to O, and the network will work fine, but may
# spike once after the w bits of output.
#
# The network is printed on standard output, and information for running is
# put into tmp_info.txt.  

if [ $# -ne 2 ]; then
  echo 'usage: sh scripts/subtraction_network.sh w os_framework' >&2
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
  echo AN 0 1 2 3 4 5 6 7 8
  echo AI 0 1 2
  echo AO 8
  echo SETNAME 0 A
  echo SETNAME 1 B
  echo SETNAME 2 S
  echo SETNAME 3 C
  echo SETNAME 4 Bias
  echo SETNAME 5 S1
  echo SETNAME 6 S2
  echo SETNAME 7 S3
  echo SETNAME 8 O

  echo SNP_ALL Threshold 1
  echo SNP 6 Threshold 2
  echo SNP 7 Threshold 3

  echo AE 0 5   0 6   0 7
  echo AE 1 5   1 6   1 7
  echo AE 2 3   2 4   2 5   2 6   2 7
  echo AE 3 4   3 8
  echo AE 4 4   4 5   4 6   4 7
  echo AE 5 8
  echo AE 6 5   6 6   6 7   6 8
  echo AE 7 8

  echo SEP_ALL Weight 1
  echo SEP_ALL Delay 1
  echo SEP 2 3 Delay $(($w-1))
  echo SEP 3 8 Delay 3
  echo SEP 1 5    1 6   1 7   Weight -1
  echo SEP 2 5    2 6   2 7   Weight 2
  echo SEP 3 4    3 8         Weight -1
  echo SEP 6 8 Weight -1

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
  echo OUTPUT DIFF 8 TC_LE $w 2 
  echo RUN $(($w+3))
) > tmp_info.txt

rm -f tmp_risp.txt
rm -f tmp_emptynet.txt
rm -f tmp_network_tool.txt

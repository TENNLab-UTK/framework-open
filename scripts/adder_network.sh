# This creates a network that adds two numbers that are fed in a binary spike train,
# in little endian.  It works with positive and negative numbers, where the negative
# numbers are converted with two's complement.
#
# w is used to isolate the output to w bits.  This is necessary, because sometimes there
# is overflow, and sometimes there is not overflow, and it is inconsistent (when you use
# two's complement for negative numbers).
#
# If you set w to zero, then it does not put in the isolation, and the network is exactly
# the same as in Aimone's paper.
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

if [ $w -ne 0 ]; then
  s=2
  s1=3
  s2=4
  s3=5
  sum=6
else
  s=BADS
  s1=2
  s2=3
  s3=4
  sum=5
fi

( echo FJ tmp_emptynet.txt
  echo AN 0 1 $s1 $s2 $s3 $sum
  echo AI 0 1
  echo AO $sum
  echo SETNAME 0 V0
  echo SETNAME 1 V1
  echo SETNAME $s1 S1
  echo SETNAME $s2 S2
  echo SETNAME $s3 S3
  echo SETNAME $sum SUM
  if [ $w -ne 0 ]; then
    echo AN $s
    echo AI $s
    echo SETNAME $s S
  fi
  echo SNP_ALL Threshold 1
  echo SNP $s2 Threshold 2
  echo SNP $s3 Threshold 3

  
  echo AE 0 $s1   0 $s2   0 $s3
  echo AE 1 $s1   1 $s2   1 $s3
  echo AE $s1 $sum
  echo AE $s2 $s1   $s2 $s2   $s2 $s3   $s2 $sum
  echo AE $s3 $sum


  echo SEP_ALL Weight 1
  echo SEP_ALL Delay 1
  echo SEP $s2 $sum Weight -1

  if [ $w -ne 0 ]; then 
    echo AE $s $sum
    echo SEP $s $sum Weight -1
    echo SEP $s $sum Delay $(($w+2))
  fi
  
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
  if [ $w -ne 0 ]; then
    echo INPUT  S   $s Spike 1    0 
  fi
  echo OUTPUT SUM $sum TC_LE $w 2 
  echo RUN $(($w+3))
) > tmp_info.txt

rm -f tmp_risp.txt
rm -f tmp_emptynet.txt
rm -f tmp_network_tool.txt

# This script takes a number and its number of bits (w), and outputs the two's
# complement negation of it as a w-bit number.

if [ $# -ne 3 ]; then
  echo 'usage: sh scripts/twos_complement_w.sh v1 bits os_framework' >&2
  exit 1
fi

v=$1
bits=$2
fro=$3

if [ $v -lt 0 ]; then echo "V cannot be negative" >&2; exit 1; fi

# Convert the number into a spike stream

w=0
sr=""
while [ $v -ne 0 ]; do
  w=$(($w+1))
  b=$(($v%2))
  v=$(($v/2))
  sr="$sr$b"
done
  
if [ $w = 0 ]; then
  w=1
  sr=0
fi

if [ $w -gt $bits ]; then echo "Not enough bits to store $v (must be at least $w)">&2 ; exit 1 ; fi

# Make an empty network with the proper RISP parameters.
# By default, leak is on.

cat $fro/params/risp_127.txt | sed '/leak_mode/s/none/all/' > tmp_risp.txt

( echo M risp tmp_risp.txt
  echo EMPTYNET tmp_emptynet.txt ) | $fro/bin/processor_tool_risp

# Now create the network and put it into tmp_network.txt

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
  echo SEP 1 2 Delay $(($bits-1))
  echo SEP 2 3 Weight -1
  echo SEP 5 6 Weight -1

  echo SORT Q
  echo TJ tmp_network.txt

  ) > tmp_network_tool.txt

$fro/bin/network_tool < tmp_network_tool.txt

echo "Input in little endian: $sr "

# Create the processor_tool input and run it.

( echo ML tmp_network.txt
  echo ASR 0 $sr
  echo AS 1 0 1
  echo RUN $(($bits+2))
  echo GSR 
) > tmp_pt_input.txt

$fro/bin/processor_tool_risp < tmp_pt_input.txt > tmp_pt_output.txt

cat tmp_pt_output.txt

# Now, construct the output from the O neuron.

osr=`tail -n 1 tmp_pt_output.txt | awk '{ print $NF }' | sed 's/..//'`
while [ `echo $osr | wc | awk '{ print $3 }'` -le $bits ]; do osr=$osr"0"; done
echo Answer in Little Endian: $osr

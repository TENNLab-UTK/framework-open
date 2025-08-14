# This compares two non-negative numbers that are fed in a binary spike train,
# in little endian.

if [ $# -ne 3 ]; then
  echo 'usage: sh scripts/aimone_comparison.sh v1 v2 os_framework' >&2
  exit 1
fi

v1=$1
v2=$2
fro=$3

if [ $v1 -lt 0 ]; then echo "V1 cannot be negative" >&2; exit 1; fi
if [ $v2 -lt 0 ]; then echo "V2 cannot be negative" >&2; exit 1; fi

# Convert the first number into a spike stream and calculate the bits

tv1=$v1
w1=0
sr1=""
while [ $tv1 -ne 0 ]; do
  w1=$(($w1+1))
  b=$(($tv1%2))
  tv1=$(($tv1/2))
  sr1="$sr1$b"
done
  
# Convert the second number into a spike stream and calculate the bits

tv2=$v2
w2=0
sr2=""
while [ $tv2 -ne 0 ]; do
  w2=$(($w2+1))
  b=$(($tv2%2))
  tv2=$(($tv2/2))
  sr2="$sr2$b"
done
  
if [ $w1 -gt $w2 ]; then w=$w1; else w=$w2; fi

# Pad the spike streams so that they are the same size

while [ $w -gt $w1 ]; do sr1=$sr1"0"; w1=$(($w1+1)); done
while [ $w -gt $w2 ]; do sr2=$sr2"0"; w2=$(($w2+1)); done

echo "Inputs in little endian: $sr1 $sr2"
echo "w: $w"

for i in network_tool processor_tool_risp compose_networks ; do
  if [ ! -x $fro/bin/$i ]; then
    ( cd $fro ; make bin/$i )
  fi
done

# Make an addition network and put it into tmp_addition.txt
# Also prepend '+' to all of the neuron names.

sh scripts/aimone_adder.sh $v1 $v2 $fro > /dev/null
sed '/SETNAME/s/ \([^ ]*\)$/ +\1/' < tmp_network_tool.txt | $fro/bin/network_tool
mv tmp_network.txt tmp_addition.txt

# Make an inversion network and put it into tmp_inversion.txt

sh scripts/inversion.sh $v1 $w $fro > /dev/null
mv tmp_network.txt tmp_inversion.txt

# Compose the two networks, conflating the O neuron of the inversion network
# with the B neuron of the adder

echo CONFLATE 1 2 | bin/compose_networks tmp_addition.txt tmp_inversion.txt "I-" > tmp_network.txt

# Get the numbers of the starting node for the inversion network (named I-S),
# and for the S2 node in the addition network (named +S2)

sid=`( echo FJ tmp_network.txt ; echo INFO ) | $fro/bin/network_tool |
  sed -n -e 's/.* \([0-9]*\)(I-S).*/\1/p'`

s2id=`( echo FJ tmp_network.txt ; echo INFO ) | $fro/bin/network_tool |
   sed -n -e 's/.* \([0-9]*\)(+S2).*/\1/p'`

# Create a network_tool input file that adds the two final neurons (the input A and the
# output O) to the network and hooks them in properly.  


( echo FJ tmp_network.txt
  echo AN 9 10
  echo AI 9
  echo AO 10
  echo SNP 9 Threshold 1
  echo SNP 10 Threshold 2
  echo SETNAME 9 A
  echo SETNAME 10 O
  echo AE 9 0
  echo SEP 9 0 Weight 1
  echo SEP 9 0 Delay 1
  echo AE $s2id 10
  echo SEP $s2id 10 Weight 1
  echo SEP $s2id 10 Delay 1
  echo AE $sid 10
  echo SEP $sid 10 Weight 1
  echo SEP $sid 10 Delay $((w+2))
  echo SORT Q
  echo TJ tmp_network.txt ) > tmp_network_tool.txt

$fro/bin/network_tool < tmp_network_tool.txt

# Create the processor_tool input and run it.

( echo ML tmp_network.txt
  echo ASR 9 $sr1
  echo ASR 6 $sr2
  echo AS 7 0 1
  echo RUN $(($w+3))
  echo GSR
) > tmp_pt_input.txt

$fro/bin/processor_tool_risp < tmp_pt_input.txt > tmp_pt_output.txt

cat tmp_pt_output.txt

# If neuron 10 spikes, then v1 > v2.  If it doesn't, then v1 < v2.
# Neuron 10 should spike at timestep w+2.  We're going to check on all of these
# things to make sure that it works.

# Grab the spike raster of the "O" neuron and put spaces between each character so
# that it's easier to process in awk.

sr=`sed -n 's/.*(O).* \(.*\)/\1/p' < tmp_pt_output.txt | sed 's/\(.\)/\1 /g'`
sum=`echo $sr | awk '{ for (i = 1; i <= NF; i++) sum += $i; print sum }'`
last=`echo $sr | awk '{ print $NF }'`

if [ $sum != $last ]; then
  echo "Error -- the O neuron fired before the last timestep." >&2
  exit 1
fi

if [ $last = 0 -a $v1 -gt $v2 ]; then
  echo "Error -- v1 ($v1) is greater than v2 $(v2), but neuron O did not spike." >&2
  exit 1
fi

if [ $last = 1 -a $v1 -le $v2 ]; then
  echo "Error -- v1 ($v1) is less than or equal to v2 ($v2), but neuron O spiked." >&2
  exit 1
fi

echo "OK -- v1: $v1, v2: $v2, last spike of O: $last"
exit 0

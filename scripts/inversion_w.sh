# This script takes a number and its number of bits, and streams it into
# the inversion network to invert its bits.  It only runs the bias neuron
# for the specified number of bits so that we don't have the bias going
# later.  We might not want this feature, but for developing networks,
# it makes my life easier, so I'm keeping it.

if [ $# -ne 3 ]; then
  echo 'usage: sh scripts/inversion.sh v1 bits os_framework' >&2
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
  
if [ $bits -lt 2 ]; then echo "The number of bits has to be >= 2" >&2 ; exit 1; fi
if [ $w -gt $bits ]; then echo "The number requires more than $bits bits" >&2 ; exit 1; fi

while [ $w -lt $bits ]; do
  sr="$sr"0
  w=$(($w+1))
done

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

# Now create the network and put it into tmp_network.txt

( echo FJ tmp_emptynet.txt
  echo AN 0 1 2 3 4
  echo AI 0 1
  echo AO 2
  echo SETNAME 0 A
  echo SETNAME 1 S
  echo SETNAME 2 O
  echo SETNAME 3 Bias
  echo SETNAME 4 C
  echo SNP_ALL Threshold 1

  echo AE 0 2 
  echo AE 1 2   1 3  1 4
  echo AE 3 3   3 2
  echo AE 4 3

  echo SEP_ALL Weight 1
  echo SEP_ALL Delay 1
  echo SEP 0 2 Weight -1
  echo SEP 1 4 Delay $(($bits-1))
  echo SEP 4 3 Weight -1
  
  echo SORT Q
  echo TJ tmp_network.txt
  
  ) > tmp_network_tool.txt

$fro/bin/network_tool < tmp_network_tool.txt

echo "Input in little endian: $sr "
echo "bits: $w"

# Create the processor_tool input and run it.

( echo ML tmp_network.txt
  echo ASR 0 $sr
  echo AS 1 0 1
  echo RSC $(($w+1))
) > tmp_pt_input.txt

$fro/bin/processor_tool_risp < tmp_pt_input.txt > tmp_pt_output.txt

cat tmp_pt_output.txt

# Now, construct the output from the O neuron.

osr=`sed 1,2d tmp_pt_output.txt | awk '{ print $4 }' | sed 's/\*/1/g' | sed 's/-/0/g'`
osr=`echo $osr | sed 's/ //g'`
echo Answer in Little Endian: $osr

# s=`echo $osr | sed 's/\(.\)/ \1/g' |
#    awk '{ for (i = NF; i > 0; i--) { v *= 2; v += $i }; print v }'`
# 
# echo Sum in Decimal: $s

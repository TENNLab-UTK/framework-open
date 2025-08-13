# This adds two numbers that are fed in a binary spike train,
# in little endian.

# Converting a V_1 value to make a sign neuron spike.

if [ $# -ne 3 ]; then
  echo 'usage: sh scripts/aimone_adder.sh v1 v2 os_framework' >&2
  exit 1
fi

v1=$1
v2=$2
fro=$3

if [ $v1 -lt 0 ]; then echo "V1 cannot be negative" >&2; exit 1; fi
if [ $v2 -lt 0 ]; then echo "V2 cannot be negative" >&2; exit 1; fi

for i in network_tool processor_tool_risp ; do
  if [ ! -x $fro/bin/$i ]; then
    ( cd $fro ; make bin/$i )
  fi
done

# Make an empty network with the proper RISP parameters.

cat $fro/params/risp_127.txt | sed '/leak_mode/s/none/all/' > tmp_risp.txt

( echo M risp tmp_risp.txt
  echo EMPTYNET tmp_emptynet.txt ) | $fro/bin/processor_tool_risp

# Now create the network and put it into tmp_network.txt

( echo FJ tmp_emptynet.txt
  echo AN 0 1 2 3 4 5
  echo AI 0 1
  echo AO 5
  echo SETNAME 0 A
  echo SETNAME 1 B
  echo SETNAME 2 S1
  echo SETNAME 3 S2
  echo SETNAME 4 S3
  echo SETNAME 5 O
  echo SNP_ALL Threshold 1
  echo SNP 3 Threshold 2
  echo SNP 4 Threshold 3

  echo AE 0 2   0 3   0 4
  echo AE 1 2   1 3   1 4
  echo AE 2 5
  echo AE 3 2   3 3   3 4   3 5
  echo AE 4 5

  echo SEP_ALL Weight 1
  echo SEP_ALL Delay 1
  echo SEP 3 5 Weight -1
  
  echo SORT Q
  echo TJ tmp_network.txt
  
  ) > tmp_network_tool.txt

$fro/bin/network_tool < tmp_network_tool.txt

# Convert the first number into a spike stream

w1=0
sr1=""
while [ $v1 -ne 0 ]; do
  w1=$(($w1+1))
  b=$(($v1%2))
  v1=$(($v1/2))
  sr1="$sr1$b"
done
  
# Convert the second number into a spike stream

w2=0
sr2=""
while [ $v2 -ne 0 ]; do
  w2=$(($w2+1))
  b=$(($v2%2))
  v2=$(($v2/2))
  sr2="$sr2$b"
done
  
if [ $w1 -gt $w2 ]; then w=$w1; else w=$w2; fi

# Pad the spike rasters so that they are the same size

while [ $w -gt $w1 ]; do sr1=$sr1"0"; w1=$(($w1+1)); done
while [ $w -gt $w2 ]; do sr2=$sr2"0"; w2=$(($w2+1)); done

echo "Inputs in little endian: $sr1 $sr2"
echo "w: $w"

# Create the processor_tool input and run it.

( echo ML tmp_network.txt
  echo ASR 0 $sr1
  echo ASR 1 $sr2
  echo RSC $(($w+3))
) > tmp_pt_input.txt

$fro/bin/processor_tool_risp < tmp_pt_input.txt > tmp_pt_output.txt

cat tmp_pt_output.txt

# Now, construct the output from the O neuron.

osr=`sed 1,3d tmp_pt_output.txt | awk '{ print $7 }' | sed 's/\*/1/g' | sed 's/-/0/g'`
osr=`echo $osr | sed 's/ //g'`
echo Sum in Little Endian: $osr

s=`echo $osr | sed 's/\(.\)/ \1/g' |
   awk '{ for (i = NF; i > 0; i--) { v *= 2; v += $i }; print v }'`

echo Sum in Decimal: $s

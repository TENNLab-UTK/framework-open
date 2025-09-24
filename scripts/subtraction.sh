# This script takes a number and its number of bits (w), and outputs the two's
# complement negation of it as a w-bit number.
#
# It's up to you to make sure that w is big enough -- we don't make it change to
# accommodate overflow.

if [ $# -ne 4 ]; then
  echo 'usage: sh scripts/subtraction.sh v1 v2 bits os_framework' >&2
  exit 1
fi

v1=$1
v2=$2
bits=$3
fro=$4

if [ $v1 -lt 0 ]; then echo "V1 cannot be negative" >&2; exit 1; fi
if [ $v2 -lt 0 ]; then echo "V2 cannot be negative" >&2; exit 1; fi

sv1=$v1
sv2=$v2

# Convert the numbers into a spike stream

w1=0
sr1=""
while [ $v1 -ne 0 ]; do
  w1=$(($w1+1))
  b1=$(($v1%2))
  v1=$(($v1/2))
  sr1="$sr1$b1"
done
  
w2=0
sr2=""
while [ $v2 -ne 0 ]; do
  w2=$(($w2+1))
  b2=$(($v2%2))
  v2=$(($v2/2))
  sr2="$sr2$b2"
done

if [ $w1 -gt $bits ]; then echo "Not enough bits to store $v1" >&2 ; exit 1 ; fi
if [ $w2 -gt $bits ]; then echo "Not enough bits to store $v2" >&2 ; exit 1 ; fi

# Make an empty network with the proper RISP parameters.
# By default, leak is on.

cat $fro/params/risp_127.txt | sed '/leak_mode/s/none/all/' > tmp_risp.txt

( echo M risp tmp_risp.txt
  echo EMPTYNET tmp_emptynet.txt ) | $fro/bin/processor_tool_risp

# Now create the network and put it into tmp_network.txt

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
  echo SEP 2 3 Delay $(($bits-1))
  echo SEP 3 8 Delay 3
  echo SEP 1 5    1 6   1 7   Weight -1
  echo SEP 2 5    2 6   2 7   Weight 2
  echo SEP 3 4    3 8         Weight -1
  echo SEP 6 8 Weight -1

  echo SORT Q
  echo TJ tmp_network.txt

  ) > tmp_network_tool.txt

$fro/bin/network_tool < tmp_network_tool.txt

echo "Input 1 in little endian: $sr1 "
echo "Input 2 in little endian: $sr2 "

# Create the processor_tool input and run it.

( echo ML tmp_network.txt
  echo ASR 0 $sr1
  echo ASR 1 $sr2
  echo AS 2 0 1
  echo RUN $(($bits+3))
  echo GSR 
) > tmp_pt_input.txt

$fro/bin/processor_tool_risp < tmp_pt_input.txt > tmp_pt_output.txt

cat tmp_pt_output.txt

# Now, construct the output from the O neuron.

osr=`tail -n 1 tmp_pt_output.txt | awk '{ print $NF }' | sed 's/..//'`
while [ `echo $osr | wc | awk '{ print $3 }'` -le $bits ]; do osr=$osr"0"; done
echo Answer in Little Endian: $osr

# Double-check the answer

diff=$(($sv1-$sv2))
if [ $diff -ge 0 ]; then
  a=""
  for i in `seq $bits` ; do
    b=$(($diff%2))
    diff=$(($diff/2))
    a=$a"$b"
  done
else 
  a=`sh $fro/scripts/twos_complement_w.sh $((-$diff)) $bits $fro | tail -n 1 | sed 's/.* //'`
fi

if [ $a = $osr ]; then
  echo Answer Corroborated
else
  echo "Answer Mismatch:" $osr $a
fi

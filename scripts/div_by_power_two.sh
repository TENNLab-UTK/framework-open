# This script takes a number and its number of bits (w), and outputs the two's
# complement negation of it as a w-bit number.

if [ $# -ne 4 ]; then
  echo 'usage: sh scripts/div_by_power_two.sh v k bits os_framework' >&2
  exit 1
fi

v=$1
k=$2
bits=$3
fro=$4
w=$bits

# Convert the number into a spike stream

if ! sh $fro/scripts/val_to_tcle.sh $v $bits > /dev/null ; then exit 1 ; fi

sr=`sh $fro/scripts/val_to_tcle.sh $v $bits`

# Generate a twos_complement inverter

sh scripts/twos_complement_w.sh $v $bits $fro > /dev/null

# Add the '>>' neuron

( echo FJ tmp_network.txt
  echo AN 7  8  9  10
  echo SNP 7 9 Threshold 2
  echo SNP 8 10 Threshold 1
  echo SETNAME 7 '>>'
  echo SETNAME 8 T1
  echo SETNAME 9 T2
  echo SETNAME 10 OT-
  echo AE  6 7   1 7   3 7
  echo SEP 6 7   1 7   3 7  Weight 1
  echo SEP 6 7              Delay 1 
  echo SEP       1 7   3 7  Delay $(($k+3))
  echo AE 1 8   1 9
  echo SEP 1 8  1 9 Weight 2
  echo SEP 1 8  1 9 Delay $(($k+4))
  echo AE 3 8   3 9
  echo SEP 3 8  3 9 Weight 1
  echo SEP 3 8  3 9 Delay $(($k+4))
  echo AE 7 8   7 9
  echo SEP 7 8  7 9 Weight -1
  echo SEP 7 8  7 9 Delay 1
  echo AE 8 10 
  echo SEP 8 10 Weight 1
  echo SEP 8 10 Delay 1
  echo AE 9 8  9 9  9 10
  echo SEP 9 8  9 9  9 10 Delay 1
  echo SEP 9 8  9 9  Weight 1
  echo SEP 9 10 Weight -1
  echo AN 11 12
  echo SETNAME 11 +SEL
  echo SETNAME 12 -SEL
  echo SNP 11 Threshold 1
  echo SNP 12 Threshold 2
  echo AE  0 11   0 12   1 11   1 12
  echo SEP 0 11                       Weight -1
  echo SEP        0 12   1 11   1 12  Weight 1
  echo SEP 0 11   0 12                Delay 1
  echo SEP               1 11   1 12  Delay $(($w-1))
  echo AN 13 14 15
  echo SETNAME 13 C+
  echo SETNAME 14 B+
  echo SETNAME 15 O+
  echo SNP 13 14 Threshold 1
  echo SNP 15 Threshold 2
  echo AE  0 15
  echo SEP 0 15 Weight 1
  echo SEP 0 15 Delay $((5+$w-$k))
  echo AE  11 13   11 14   13 14   14 14   14 15
  echo SEP 11 13   11 14           14 14   14 15 Weight 1
  echo SEP                 13 14                        Weight -1
  echo SEP         11 14   13 14   14 14                Delay 1
  echo SEP 11 13                                        Delay $w
  echo SEP                                        14 15 Delay 5
  echo AN 16 17 18
  echo SETNAME 16 C-
  echo SETNAME 17 B-
  echo SETNAME 18 O-
  echo SNP 16 17 Threshold 1
  echo SNP 18    Threshold 2
  echo AE  12 16   12 17   16 17   17 17   17 18   10 18 
  echo SEP 12 16                                          Delay $w
  echo SEP         12 17   16 17   17 17                  Delay 1
  echo SEP                                         10 18  Delay $(($w-$k))
  echo SEP                                 17 18          Delay 5
  echo SEP 12 16   12 17           17 17   17 18   10 18  Weight 1
  echo SEP                 16 17                          Weight -1
  echo AN 19
  echo SETNAME 19 Q
  echo SNP 19 Threshold 1
  echo AE 15 19   18 19
  echo SEP 15 19   18 19 Weight 1 
  echo SEP 15 19   18 19 Delay 1 


  echo TJ tmp_network.txt ) | $fro/bin/network_tool

echo "Input in little endian: $sr "

# Create the processor_tool input and run it.

( echo ML tmp_network.txt
  echo ASR 0 $sr
  echo AS 1 0 1
  echo RUN $((100))
  echo GSR 
) > tmp_pt_input.txt

$fro/bin/processor_tool_risp < tmp_pt_input.txt > tmp_pt_output.txt

cat tmp_pt_output.txt

# Now, construct the output from the O neuron.

# osr=`tail -n 1 tmp_pt_output.txt | awk '{ print $NF }' | sed 's/..//'`
# while [ `echo $osr | wc | awk '{ print $3 }'` -le $bits ]; do osr=$osr"0"; done
# cho Answer in Little Endian: $osr

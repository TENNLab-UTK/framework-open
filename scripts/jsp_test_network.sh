# Please see

if [ $# -ne 9 ]; then
  echo "usage: sh scripts/jsp_test_network.sh a b c d w m t processor proc-params" >&2
  exit 1
fi

a=$1
b=$2
c=$3
d=$4
w=$5
m=$6
t=$7
processor=$8
params=$9

pt=bin/processor_tool_$processor         # Processor tool executable
nt=bin/network_tool                       # Network tool executable

# Make sure you have the executables that you need.

for i in $pt $nt $params ; do
  if [ ! -f $i ]; then
    echo "You need the program/file $i" >&2
    exit 1
  fi
done

# Test to make sure that the processor parameters are legal

if [ `echo M $processor $params | $pt 2>&1 | wc | awk '{ print $1 }'` != 0 ]; then
  echo M $processor $params | $pt
  exit 1
fi

# Make an empty network for the processor
  
empty=tmp_"$processor"_empty.txt
( echo M $processor $params ; echo EMPTYNET $empty ) | $pt

# Figure out the minimum synapse delay

msd=`( echo FJ $empty ; echo P ) | $nt |
   sed '1,/edge_properties/d' | sed '/network_properties/,$d' |
   sed -n 's/.*name.:.Delay.,.*min_value.:\([^,]*\).*/\1/p' | awk '{ printf "%g\n", $1 }'`

# Create the network file

( echo FJ $empty
  echo AN 0 1 2 3 4
  echo SETNAME 0 Main
  echo SETNAME 1 On
  echo SETNAME 2 Off
  echo SETNAME 3 Out
  echo SETNAME 4 Bias

  echo AI 0 1 2
  echo AO 3 4 

  echo SNP 0 1 2 4 Threshold 1
  echo SNP 3 Threshold $t

  echo AE  0 4  4 4
  echo SEP 0 4  4 4 Delay $msd
  echo SEP 0 4  4 4 Weight $w

  echo AE  1 0  
  echo SEP 1 0 Delay $a
  echo SEP 1 0 Weight $w

  echo AE  2 0  
  echo SEP 2 0 Delay $b
  echo SEP 2 0 Weight "-"$w

  echo AE  0 0  
  echo SEP 0 0 Delay $c
  echo SEP 0 0 Weight $w

  echo AE  0 3  
  echo SEP 0 3 Delay $d
  echo SEP 0 3 Weight $m

  echo SORT Q
  echo TJ ) | $nt 

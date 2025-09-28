# This creates a bit-inversion network in tmp_inversion.txt.  It takes a value to invert,
# and a number of bits (w).  It reads the information in tmp_info.txt to create
# processor_tool commands to run the inversion network.  It 
# runs the network, and then double-checks the results.

if [ $# -ne 3 ]; then
  echo 'usage: sh scripts/inversion_run.sh v w os_framework' >&2
  exit 1
fi

v=$1
w=$2
fro=$3

# Do error checking 

len=`echo $v | wc | awk '{ print $3-1 }'`
if [ $len != $w ]; then
  echo "V's length ($len) is not equal to w ($w)" >&2
  exit 1
fi

if [ `echo $v | sed 's/0/1/g' | sed 's/1*/1/'` != 1 ]; then
  echo "Bad v - $v - must be composed only of zeros and ones" >&2
  exit 1
fi

ld=`echo $v | sed 's/.*\(.\)$/\1/'`
c=`echo $v | sed 's/0/A/g' | sed 's/1/0/g' | sed 's/A/1/g'`

# Make the inversion network.  Info is going to be in tmp_info.txt.

sh $fro/scripts/inversion_network.sh $w $fro > tmp_inversion.txt

# Get the starting timestep and neurons of v0 and v1.
# Get the running time
# Get the output timesteps
# Get the output starting time

nv0=`grep V0 tmp_info.txt | awk '{ print $3 }'`
run=`grep RUN tmp_info.txt | awk '{ print $2 }'`
on=`grep OUTPUT tmp_info.txt | awk '{ print $3 }'`
ots=`grep OUTPUT tmp_info.txt | awk '{ print $5 }'`
ost=`grep OUTPUT tmp_info.txt | awk '{ print $6 }'`
s=`grep ' S ' tmp_info.txt | awk '{ print $3 }'`

# Create the processor_tool input and run it.

( echo ML tmp_inversion.txt
  echo ASR $nv0 $v
  echo AS $s 0 1
  echo RUN $run
  echo GSR
) > tmp_pt_input.txt

$fro/bin/processor_tool_risp < tmp_pt_input.txt > tmp_pt_output.txt

dots=`echo $ots | awk '{ for (i = 0; i < $1; i++) printf "."; print "" }'`
st_dots=`echo $ost | awk '{ for (i = 0; i < $1; i++) printf "."; print "" }'`
zeros=`echo "$dots$st_dots" | sed 's/./0/g'`

output=`grep '^'$on'(' tmp_pt_output.txt | awk '{ print $NF }'`
stripped=`echo "$output$zeros" | sed 's/'$st_dots'\('$dots'\).*/\1/'`

if [ $stripped = $c ]; then correct=1; else correct=0; fi
echo "V0: $v"
echo "W: $w"
echo "Output-Neuron: $on"
echo "Output-Starting-Timestep: $ost"
echo "Output-Num-Timesteps: $ots"
echo "Output-On-Output-Neuron: $output"
echo "Stripped-Output: $stripped"
echo "Input-Inverted: $c"
echo "Correct: $correct"

echo ""
echo "The network is in tmp_inversion.txt"
echo "Its info is in tmp_info.txt"
echo "Input for the processor_tool to run this test is in tmp_pt_input.txt"
echo "Output of the processor_tool on this input is in tmp_pt_output.txt"


# This creates a network to do two's complement inversion and puts it into
# tmp_twos_comp.txt .  It takes a value to invert,
# and a number of bits (w).  It reads the information in tmp_info.txt to create
# processor_tool commands to run the twos_complement network.  It 
# runs the network, and then double-checks the results.

if [ $# -ne 3 ]; then
  echo 'usage: sh scripts/inversion_run.sh v w os_framework' >&2
  exit 1
fi

v=$1
w=$2
fro=$3

# Error check v, and turn it into two's complement, little endian.

if ! sh $fro/scripts/val_to_tcle.sh $v $w > tmp_v.txt ; then exit 1 ; fi
sr=`cat tmp_v.txt`

# Invert it, and get the spike raster of the inversion

c=$((-$v))
if ! sh $fro/scripts/val_to_tcle.sh $c $w > tmp_c.txt ; then exit 1 ; fi
src=`cat tmp_c.txt`

# Make the twos_comp network.  Info is going to be in tmp_info.txt.

sh $fro/scripts/twos_comp_network.sh $w $fro > tmp_twos_comp.txt

# Get the starting timestep and neurons of v
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

( echo ML tmp_twos_comp.txt
  echo ASR $nv0 $sr
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

if [ $stripped = $src ]; then correct=1; else correct=0; fi
echo "V0: $v"
echo "W: $w"
echo "V0-Spike-Raster: $sr"
echo "Output-Neuron: $on"
echo "Output-Starting-Timestep: $ost"
echo "Output-Num-Timesteps: $ots"
echo "Output-On-Output-Neuron: $output"
echo "Stripped-Output: $stripped"
echo "Input-Inverted: $c"
echo "Inverted-Spike-Raster: $src"
echo "Correct: $correct"

echo ""
echo "The network is in tmp_twos_comp.txt"
echo "Its info is in tmp_info.txt"
echo "Input for the processor_tool to run this test is in tmp_pt_input.txt"
echo "Output of the processor_tool on this input is in tmp_pt_output.txt"


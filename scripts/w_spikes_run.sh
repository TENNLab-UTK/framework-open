# This creates a network that spikes w times, and then runs it.

if [ $# -ne 2 ]; then
  echo 'usage: sh scripts/w_spikes_run.sh w os_framework' >&2
  exit 1
fi

w=$1
fro=$2

# Compile necessary binaries

for i in network_tool processor_tool_risp ; do
  if [ ! -x $fro/bin/$i ]; then
    ( cd $fro ; make bin/$i )
  fi
done

# Make the network.  Info is going to be in tmp_info.txt.

sh $fro/scripts/w_spikes_network.sh $c $w $fro > tmp_w_spikes.txt

# Get the starting neuron 
# Get the running time
# Get the output timesteps
# Get the output starting time

s=`grep ' S ' tmp_info.txt | awk '{ print $3 }'`
run=`grep RUN tmp_info.txt | awk '{ print $2 }'`
on=`grep OUTPUT tmp_info.txt | awk '{ print $3 }'`
ots=`grep OUTPUT tmp_info.txt | awk '{ print $5 }'`
ost=`grep OUTPUT tmp_info.txt | awk '{ print $6 }'`

# Create the processor_tool input and run it.

( echo ML tmp_w_spikes.txt
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
w_spikes=`sh $fro/scripts/tcle_to_val.sh $stripped`

top=`echo $w | awk '{ i=1 ; for (j = 1; j < $1; j++) i *= 2; print i }'`

if [ $w_spikes = -1 ]; then correct=1 ; else correct=0; fi

echo "C: $c"
echo "W: $w"
echo "Top: $top"
echo "Output-Neuron: $on"
echo "Output-Starting-Timestep: $ost"
echo "Output-Num-Timesteps: $ots"
echo "Output-On-Output-Neuron: $output"
echo "Stripped-Output: $stripped"
echo "Computed-Value: $w_spikes"
echo "Correct: $correct"

echo ""
echo "The network is in tmp_w_spikes.txt"
echo "Its info is in tmp_info.txt"
echo "Input for the processor_tool to run this test is in tmp_pt_input.txt"
echo "Output of the processor_tool on this input is in tmp_pt_output.txt"

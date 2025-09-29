# This creates an mult/const network in tmp_mult.txt.  It takes a constant for the
# multiplier, a value to multiply,
# and a number of bits (w).  It reads the information in tmp_info.txt to create
# processor_tool commands to run the multiplier.  It converts v0 to little endian
# two's complement, runs the network, and then double-checks the results.
#
# We're going with fixed bit-width here, so you need to be really careful with
# negative numbers and overflow.  I'll deal with the documentation of that later.

if [ $# -ne 4 ]; then
  echo 'usage: sh scripts/aimone_mult_run.sh const v0 w os_framework' >&2
  exit 1
fi

const=$1
v0=$2
w=$3
fro=$4

# Error check const and v0. Turn v0 into two's complement, little endian.

if ! sh $fro/scripts/val_to_tcle.sh $v0 $w > tmp_v0.txt ; then exit 1 ; fi
if ! sh $fro/scripts/val_to_tcle.sh $const $w > /dev/null ; then exit 1 ; fi

sr0=`cat tmp_v0.txt`

# Compile necessary binaries

for i in network_tool processor_tool_risp ; do
  if [ ! -x $fro/bin/$i ]; then
    ( cd $fro ; make bin/$i )
  fi
done

# Make the multiplier network.  Info is going to be in tmp_info.txt.

sh $fro/scripts/aimone_mult_network.sh $const $w $fro > tmp_mult.txt

# Get the starting timestep and neurons of v0.
# Get the running time.
# Get the output timesteps.
# Get the output starting time.

nv0=`grep V0 tmp_info.txt | awk '{ print $3 }'`
run=`grep RUN tmp_info.txt | awk '{ print $2 }'`
on=`grep OUTPUT tmp_info.txt | awk '{ print $3 }'`
ots=`grep OUTPUT tmp_info.txt | awk '{ print $5 }'`
ost=`grep OUTPUT tmp_info.txt | awk '{ print $6 }'`
s=`grep ' S ' tmp_info.txt | awk '{ print $3 }'`

# Create the processor_tool input and run it.

( echo ML tmp_mult.txt
  echo ASR $nv0 $sr0
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
prod=`sh scripts/tcle_to_val.sh $stripped`

top=`echo $w | awk '{ i=1 ; for (j = 1; j < $1; j++) i *= 2; print i }'`

cs=$(($v0*$const))
if [ $cs -ge $top ]; then overflow=1; else overflow=0; fi
if [ $cs -lt $((-$top)) ]; then underflow=1; else underflow=0; fi
if [ $cs -eq $prod ]; then correct=1 ; else correct=0 ; fi

echo "V0: $v0"
echo "W: $w"
echo "Top: $top"
echo "V0-SR: $sr0"
echo "Output-Neuron: $on"
echo "Output-Starting-Timestep: $ost"
echo "Output-Num-Timesteps: $ots"
echo "Output-On-Output-Neuron: $output"
echo "Stripped-Output: $stripped"
echo "Product:" $cs
echo "Computed-Product: $prod"
echo "Correct: $correct"
echo "Overflow: $overflow"
echo "Underflow: $underflow"

echo ""
echo "The network is in tmp_adder.txt"
echo "Its info is in tmp_info.txt"
echo "Input for the processor_tool to run this test is in tmp_pt_input.txt"
echo "Output of the processor_tool on this input is in tmp_pt_output.txt"

# This script creates a network and example processor_tool commands
# that will convert an input spike with a given input value into a
# temporally encoded output value.

if [ $# -ne 2 ]; then
  echo 'usage: sh scripts/val_to_temporal.sh max_value framework-open-directory' >& 2
  exit 1
fi

max=$1
fro=$2

if [ $max -le 0 -o $max -gt 126 ]; then
  echo 'max_value has to be between 0 and 126' >&2
  exit 1
fi

if [ ! -f $fro/params/risp_127.txt ]; then
  echo 'Bad framework-open-directory.  It should have params/risp_127.txt' >&2 
  exit 1
fi

if [ ! -x $fro/bin/network_tool ]; then ( cd $fro ; make bin/network_tool ) > /dev/null 2>&2; fi
if [ ! -x $fro/bin/processor_tool_risp ]; then 
  ( cd $fro ; make bin/processor_tool_risp ) > /dev/null 2>&2
fi

( echo M risp params/risp_127.txt
  echo EMPTYNET tmp_empty_network.txt ) | $fro/bin/processor_tool_risp

( echo FJ tmp_empty_network.txt
  echo AN 0 1
  echo AI 0 1
  echo AO 0
  echo SETNAME 0 Val_v
  echo SETNAME 1 S
  echo SNP 0 Threshold $(($max+1))
  echo SNP 1 Threshold 1
  echo AE 0 0  0 1   1 0  1 1
  echo SEP_ALL Delay 1
  echo SEP_ALL Weight 1
  echo SEP 0 0  0 1 Weight -1
  echo TJ tmp_network.txt ) > tmp_network_tool.txt

$fro/bin/network_tool < tmp_network_tool.txt

echo "The network tool commands to create this network are in tmp_network_tool.txt."
echo "The network itself is in tmp_network.txt."
echo "Please run the following processor tool commands to exemplify."
echo "Do RSC instead of RUN to see the neuron charges and the spikes."
echo "----------------------------------------"

e=1
for v in 0 $max $(($max/2)) ; do
  echo "Example $e: Val = $v"
  echo "Do the following, and the result should be that neuron 0 fires at time" $(($max-v+1)) "."
  echo "Do RSC instead of RUN to see the neuron charges and the spikes."
  echo "( echo ML tmp_network.txt "
  echo "  echo ASV 0 0 $v"
  echo "  echo ASV 1 0 1"
  echo "  echo RUN $(($max+2)) "
  echo "  echo OT ) | $fro/bin/processor_tool_risp"

  e=$(($e+1))
  echo ""
  
done

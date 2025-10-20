# This creates a network that spikes a constant value in TC_LE.
#
# James S. Plank.  October, 2025.
#
# The network is printed on standard output, and information for running is
# put into tmp_info.txt.  

if [ $# -ne 3 ]; then
  echo 'usage: sh scripts/constant_network.sh c w os_framework' >&2
  exit 1
fi

c=$1
w=$2
fro=$3

# Compile necesssary programs in the open-source framework

for i in network_tool processor_tool_risp ; do
  if [ ! -x $fro/bin/$i ]; then ( cd $fro ; make bin/$i ) fi
done

# Get the spike raster of the constant

if ! sr=`sh $fro/scripts/val_to_tcle.sh $c $w` ; then
  exit 1
fi

# Create the neuron ids and the delay from s

ids=""
id=0
delay=0
for i in `echo $sr | sed 's/\(.\)/\1 /g'` ; do
  if [ $i = 1 ]; then ids="$ids $id-$delay"; id=$(($id+1)); fi
  delay=$(($delay+1))
done

# Make an empty network with the proper RISP parameters.

cat $fro/params/risp_127.txt | sed '/leak_mode/s/none/all/' > tmp_risp.txt

( echo M risp tmp_risp.txt
  echo EMPTYNET tmp_emptynet.txt ) | $fro/bin/processor_tool_risp

# Make the network

o=$id

( echo FJ tmp_emptynet.txt
  echo AN 0 $o
  echo AI 0 
  echo AO $o
  echo SETNAME 0 S
  echo SETNAME $id C

  for i in $ids ; do
    id=`echo $i | sed 's/-.*//'`
    delay=`echo $i | sed 's/.*-//'`
    if [ $id = 0 ]; then
      echo AE 0 $o
      echo SEP 0 $o Delay 1
    else
      echo AN $id
      echo AE 0 $id
      echo AE $id $o
      echo SEP 0 $id Delay $delay
      echo SEP $id $o Delay 1
    fi
  done
    
  echo SNP_ALL Threshold 1
  echo SEP_ALL Weight 1

  echo SORT Q
  echo TJ
  ) > tmp_network_tool.txt

$fro/bin/network_tool < tmp_network_tool.txt

( echo INPUT  S   0 Spike 1    0 
  echo OUTPUT C  $o TC_LE $w   1
  echo RUN $(($w+1))
) > tmp_info.txt

# rm -f tmp_risp.txt
# rm -f tmp_emptynet.txt
# rm -f tmp_network_tool.txt

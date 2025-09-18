# This multiplies a number, spike in as little-endian binary, by a constant that is baked
# into the SNN.

if [ $# -ne 3 ]; then
  echo 'usage: sh scripts/aimone_adder.sh const val os_framework' >&2
  exit 1
fi

const=$1
v=$2
fro=$3

if [ $const -le 0 ]; then echo "Const must be positive" >&2; exit 1; fi
if [ $v -lt 0 ]; then echo "V cannot be negative" >&2; exit 1; fi

for i in network_tool processor_tool_risp compose_networks ; do
  if [ ! -x $fro/bin/$i ]; then
    ( cd $fro ; make bin/$i )
  fi
done

# Turn const into litle endian (le) and count the ones (o).

le=""
o=0
x=$const
while [ $x -ne 0 ]; do
  bit=$((x%2))
  o=$(($o+$bit))
  le="$le$bit"
  x=$(($x/2))
done
  
# Make a network with a single input and a single output.  Don't connect them.
# Put this into tmp_base_network.txt

cat $fro/params/risp_127.txt | sed '/leak_mode/s/none/all/' > tmp_risp.txt

( echo M risp tmp_risp.txt
  echo EMPTYNET tmp_emptynet.txt ) | $fro/bin/processor_tool_risp

# Now create the network and put it into tmp_network.txt

( echo FJ tmp_emptynet.txt
  echo AN 0 1 
  echo AI 0
  echo AO 1
  echo SETNAME 0 I
  echo SETNAME 1 O
  echo SNP_ALL Threshold 1

  echo SORT Q
  echo TJ tmp_base_network.txt
  
  ) > tmp_network_tool_1.txt

$fro/bin/network_tool < tmp_network_tool_1.txt

# Delay is the delay to the first neuron.  It has to be at least one, since synapses
# have minimum delays of one.

delay=`echo $le | sed 's/1.*//' | wc | awk '{ print $3 }'`

# If there is only one bit in $le, then your network is done -- simply add a
# neuron with the proper delay (This will have to be figured out).

if [ $o = 1 ]; then
  
  ( echo FJ tmp_base_network.txt
    echo AE 0 1
    echo SEP 0 1 Weight 1
    echo SEP 0 1 Delay $delay
    echo SORT Q
    echo TJ tmp_network.txt ) > tmp_network_tool_2.txt
  
  $fro/bin/network_tool < tmp_network_tool_2.txt
  o_timestep=1
  cw=$delay
  input_neuron=0

else 

  # Make an adder network and move it to tmp_adder_network.txt
  
  sh $fro/scripts/aimone_adder.sh $const $v $fro > /dev/null
  mv tmp_network.txt tmp_adder_network.txt
  
  # Now, you are going to iterate o-1 times, because you need o-1 adder networks.

  cw=`echo $le | wc | awk '{ print $3 }'`

  # We are going to build the networks from back to front, and we're only going to
  # conflate the output of the new one to the A neuron of the previous.
  
  for i in `seq $(($o-1))` ; do
    if [ $i = 1 ]; then
      cp tmp_adder_network.txt tmp_base_network.txt
    else
      echo CONFLATE 5 0 |
        bin/compose_networks tmp_adder_network.txt tmp_base_network.txt $i"_" > tmp_network.txt
      mv tmp_network.txt tmp_base_network.txt
    fi
  done

  # Let's calculate the delays to the inputs.
  # The delay to the first input will be in fdelay.
  # The delays to the others will be rdelay
  
  delay=0
  tle=$le
  for i in `seq $o` ; do
    extra_delay=`echo $tle | sed 's/1.*//' | wc | awk '{ print $3 }'`
    delay=$(($delay+$extra_delay))
    # echo $i $extra_delay $delay $tle
    tle=`echo $tle | sed 's/^0*1//'`
    if [ $i = 1 ]; then 
      fdelay=$delay
    elif [ $i = 2 ]; then
      rdelay=$delay
    else 
      rdelay=$rdelay" "$delay
    fi
  done

  # Now, we're going to grab the id's of the inputs.
  # We'll do this by grabbing all of the inputs using the network tool, and
  # then deleting any that have slashes, since they were conflated.

  inputs="" 
  for i in `( echo FJ tmp_base_network.txt ; echo INFO ) | bin/network_tool | grep Input` ; do
    if [ `echo $i | sed 's/^[0-9][0-9]*([^\/]*)/XXX/'` = XXX ]; then 
      n=`echo $i | sed 's/(.*//'`
      inputs=$inputs" "$n
    fi
  done

  # echo $inputs

  # Add synapses from the A neuron to all of the other inputs, with the proper delays.
  # If fdelay is > 1, then we need to add a new input neuron with a delay of fdelay-1
  # to the original input.

  i0=`echo $inputs | sed 's/ .*//'`
  inputs=`echo $inputs | sed 's/[0-9]* //'`
  inc=0

  ( echo FJ tmp_base_network.txt
    for i in $rdelay ; do
      input=`echo $inputs | sed 's/ .*//'`
      inputs=`echo $inputs | sed 's/[0-9]* //'`
      echo AE $i0 $input
      echo SEP $i0 $input Weight 1
      echo SEP $i0 $input Delay $(($i-$fdelay+$inc))
      inc=$(($inc+2))
    done
    echo SORT Q
    echo TJ tmp_network.txt ) | bin/network_tool

  o_timestep=$((2*($o-1)))

  # Finally, if $fdelay > 1, then we need to create a new input neuron, and put a synapse
  # from it to neuron 0, because we need a delay of ($fdelay-1) going from the input to 
  # node 0.

  if [ $fdelay = 1 ]; then
    input_neuron=0
  else
    max=`( echo FJ tmp_network.txt 
           echo SORT Q
           echo NODES ) | bin/network_tool | tail -n 1 | sed 's/.*id"://' | sed 's/,.*//'`
    echo max $max
    ( echo FJ tmp_network.txt
      echo AN $(($max+1))
      echo SNP $(($max+1)) Threshold 1
      echo AI $(($max+1))
      echo AE $(($max+1)) 0
      echo SEP $(($max+1)) 0 Weight 1
      echo SEP $(($max+1)) 0 Delay $(($fdelay-1)) 
      echo SORT Q
      echo TJ tmp_network.txt ) | bin/network_tool
    input_neuron=$(($max+1))
  fi
fi

# Convert v into a spike stream, and set $w to be the number of bits.

w=0
sr=""
while [ $v -ne 0 ]; do
  w=$(($w+1))
  b=$(($v%2))
  v=$(($v/2))
  sr="$sr$b"
done
  
echo "Const in little endian: $le"
echo "V in little endian: $sr"
echo "Timestep that output begins: $o_timestep"

# Create the processor_tool input and run it.

( echo ML tmp_network.txt
  echo ASR $input_neuron $sr
  echo RUN $(($w+$o_timestep+$cw-1))
  echo GSR 
) > tmp_pt_input.txt

$fro/bin/processor_tool_risp < tmp_pt_input.txt > tmp_pt_output.txt

cat tmp_pt_output.txt

# Now, construct the output from the O neuron.

dots=""
for i in `seq $o_timestep` ; do dots="$dots". ; done

# There should only be one output neuron -- grab its spike raster and
# convert it to a number.

osr=`grep ' OUTPUT ' tmp_pt_output.txt | awk '{ print $NF }' | sed "s/$dots//"`

echo Product in Little Endian: $osr

s=`echo $osr | sed 's/\(.\)/ \1/g' |
   awk '{ for (i = NF; i > 0; i--) { v *= 2; v += $i }; print v }'`

echo Product in Decimal: $s

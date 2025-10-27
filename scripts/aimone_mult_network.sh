# This multiplies a number, spiked in as little-endian binary, by a constant that is baked
# into the SNN.   I will make two major mods to this:
#
# 1. Instead of having this be a linear cascade of adders, I can make it tree, and
#    that will cut down on some timesteps.  Not too many (log w to log log w), but
#    it would take, say, 32 down to 5.
#
# 2. I'll tweak it to go general purpose multiplication.  This will take a minute.
#
# Also, I need to double-check how this works with negative numbers.  The multiplication
# is ok, but we need to figure out how to truncate the bits. It will be a bit of a headache,
# unless we simply restrict the bit width (which we can restrict to be really large).

if [ $# -ne 3 ]; then
  echo 'usage: sh scripts/aimone_adder.sh const w os_framework' >&2
  exit 1
fi

const=$1
w=$2
fro=$3

for i in network_tool processor_tool_risp compose_networks ; do
  if [ ! -x $fro/bin/$i ]; then
    ( cd $fro ; make bin/$i )
  fi
done

# Turn const into two's complement litle endian (sr) and count the ones (o).

if ! sh $fro/scripts/val_to_tcle.sh $const $w > tmp_const.txt ; then exit 1 ; fi
sr=`cat tmp_const.txt`

o=`echo $sr | sed 's/\(.\)/\1 /g' | awk '{ for (i = 1; i <= NF; i++) sum += $i; print sum }'`

if [ $o = 0 ]; then echo 'Const cannot be zero' >&2; exit 1; fi

# Empty network into tmp_emptynet.txt

cat $fro/params/risp_127.txt | sed '/leak_mode/s/none/all/' > tmp_risp.txt

( echo M risp tmp_risp.txt
  echo EMPTYNET tmp_emptynet.txt ) | $fro/bin/processor_tool_risp

# Make a network with two inputs (const and start) and a single output.
# The output will be connected
# to an AND network so that the W bits of output get isolated.  Unfortunately, we don't
# know the delay to that output yet, so we'll set it to one, and change it later.
# We also don't connect the input to anything yet -- we'll either set up a single synapse
# (when o=1) or cascading adder networks.
#
# Put this into tmp_base_network.txt.

( echo FJ tmp_emptynet.txt
  echo AN 0 1 2 3 4
  echo AI 0 1
  echo AO 4
  echo SETNAME 0 V0
  echo SETNAME 1 Start
  echo SETNAME 2 C
  echo SETNAME 3 Bias
  echo SETNAME 4 Prod
  echo SNP_ALL Threshold 1
  echo SNP 4 Threshold 2
  echo AE 1 2    1 3
  echo AE 2 3
  echo AE 3 3    3 4
  echo SEP_ALL Delay 1
  echo SEP_ALL Weight 1
  echo SEP 1 2 Delay $w
  echo SEP 2 3 Weight -1
  echo SORT Q
  echo TJ tmp_base_network.txt
  
  ) > tmp_network_tool_1.txt

$fro/bin/network_tool < tmp_network_tool_1.txt

# If there is only one bit in $sr, then your network is pretty simple -- add a
# neuron with the proper delay.  That is "delay" plus one, because the spikes from
# the bias can't start arriving until timestep 2.  I know I can fix that by having
# a spike from S to the output, and then making the delay from S to C be w-1, but
# I'm keeping it simple.

if [ $o = 1 ]; then
  
  # Delay is the delay to the first neuron.  It has to be at least one, since synapses
  # have minimum delays of one (wc val #3 prints the size of the string + 1).

  delay=`echo $sr | sed 's/1.*//' | wc | awk '{ print $3 }'`

  ( echo FJ tmp_base_network.txt
    echo AE 0 4
    echo SEP 0 4 Weight 1
    echo SEP 0 4 Delay $(($delay+1))
    echo SORT Q
    echo TJ ) > tmp_network_tool_2.txt
  
  $fro/bin/network_tool < tmp_network_tool_2.txt

  ( echo INPUT  V0  0 TC_LE $w   0
    echo INPUT  S   1 Spike 1    0
    echo OUTPUT Prod 4 TC_LE $w  2
    echo RUN $(($w+$delay)) 
  ) > tmp_info.txt

else 

  # Make an adder network and move it to tmp_adder_network.txt.  Call adder_network with w=0,
  # because we don't have any S neuron.  This is
  # because we don't care about isolating the bits -- that's done with S/C/Bias in this 
  # network.
  
  sh $fro/scripts/adder_network.sh 0 $fro > tmp_adder_network.txt
  output_neuron=`grep SUM tmp_info.txt | awk '{ print $3 }'`
  
  # Now, you are going to iterate o-1 times, because you need o-1 adder networks.

  cw=`echo $sr | wc | awk '{ print $3 }'`

  # We are going to build the networks from back to front, and we're only going to
  # conflate the output of the new one to the A neuron of the previous.
  
  for i in `seq $(($o-1))` ; do
    if [ $i = 1 ]; then
      cp tmp_adder_network.txt tmp_base_adder.txt
    else
      echo CONFLATE $output_neuron 0 |
        $fro/bin/compose_networks tmp_adder_network.txt tmp_base_adder.txt $i"_" > tmp_network.txt
      mv tmp_network.txt tmp_base_adder.txt
    fi
  done

  # Let's calculate the delays to the inputs.
  # These are going to be in the variable rdelays
  # The first of these is delay to A (min=1 -- so it's number of leading 0 bits plus 1)
  # The rest are relative delays to the previous.
  
  delay=0
  tsr=$sr
  for i in `seq $o` ; do
    extra_delay=`echo $tsr | sed 's/1.*//' | wc | awk '{ print $3 }'`
    delay=$(($delay+$extra_delay))
    tsr=`echo $tsr | sed 's/^0*1//'`
    if [ $i = 1 ]; then 
      rdelay=$delay
      fdelay=$delay
    else 
      rdelay=$rdelay" "$(($delay-$lastdelay))
    fi
    lastdelay=$delay
  done

  # echo "Rdelay": $rdelay

  # Now, we're going to grab the id's of the inputs.
  # We'll do this by grabbing all of the inputs using the network tool, and
  # then deleting any that have slashes, since they were conflated.

  inputs="" 
  for i in `( echo FJ tmp_base_adder.txt ; echo INFO ) | $fro/bin/network_tool | grep Input` ; do
    if [ `echo $i | sed 's/^[0-9][0-9]*([^\/]*)/XXX/'` = XXX ]; then 
      n=`echo $i | sed 's/(.*//'`
      inputs=$inputs" "$n
    fi
  done

  # echo $inputs 

  # Put the synapses from input i to input i+1.  The delay will be rdelays for the first one,
  # or rdelays+1 for the remainer (since they are from adder to adder)
  # Also, add a final output neuron that we'll conflate with the output neuron from the
  # base network.

  # Get the output of the adders.

  out=`( echo FJ tmp_base_adder.txt ; echo INFO ) |
     $fro/bin/network_tool | sed -n '/Output nodes/s/.* \([0-9]\)/\1/p' | sed 's/(.*//'`

  # Get the highest neuron id so we can add a neuron with a higher id.

  highnode=`( echo FJ tmp_base_adder.txt ; echo INFO ) |
     $fro/bin/network_tool | grep 'nodes:' | sed 's/.*://' | sed 's/([^)]*)//g' |
     awk '{ for (i = 1; i <= NF; i++) print $i }' | sort -nr | head -n 1`
  first_neuron=`echo $inputs | awk '{ print $1 }'`


  ( 
    echo FJ tmp_base_adder.txt
    for i in `seq $o` ; do
      neuron=`echo $inputs | awk '{ print $'$i' }'`
      delay=`echo $rdelay | awk '{ print $'$i' }'`
      if [ $i -gt 1 ]; then
        if [ $i = 2 ]; then d=$delay ; else d=$(($delay+2)) ; fi
        echo AE $last_neuron $neuron
        echo SEP $last_neuron $neuron Weight 1
        echo SEP $last_neuron $neuron Delay $d
      fi
      last_neuron=$neuron
    done
    echo AN $(($highnode+1))
    echo SNP $(($highnode+1)) Threshold 2
    echo AE $out $(($highnode+1))
    echo SEP $out $(($highnode+1)) Delay 1
    echo SEP $out $(($highnode+1)) Weight 1
    echo SORT Q
    echo TJ tmp_base_adder.txt ) | $fro/bin/network_tool

  # Hook this network into the initial one.

  out=`( echo FJ tmp_base_adder.txt ; echo INFO ) |
     $fro/bin/network_tool | sed -n '/Output nodes/s/.* \([0-9]\)/\1/p' | sed 's/(.*//'`
 
  ( echo SYNAPSE 0 $first_neuron 1 1
    echo CONFLATE 4 $(($highnode+1)) ) |
      $fro/bin/compose_networks tmp_base_network.txt tmp_base_adder.txt "A" > tmp_network.txt
  mv tmp_network.txt tmp_base_network.txt

  av0=`( echo FJ tmp_base_network.txt ; echo INFO ) |
     $fro/bin/network_tool | sed -n 's/.* \([0-9]*\)(AV0).*/\1/p'`
 
  # Finally, fix the delay on the bias-to-output, and on the input-to-A.

  ( echo FJ tmp_base_network.txt
    echo SEP 3 4 Delay $(($o*2-1))
    echo SEP 0 $av0 Delay $fdelay
    echo SORT Q
    echo TJ ) | $fro/bin/network_tool

  # Print info

  ( echo INPUT  V0  0 TC_LE $w   0
    echo INPUT  S   1 Spike 1    0
    echo OUTPUT Prod 4 TC_LE $w $(($o*2))
    echo RUN $(($o*2+$w*2))
  ) > tmp_info.txt
fi

rm -f tmp_risp.txt
rm -f tmp_const.txt
rm -f tmp_emptynet.txt
rm -f tmp_network_tool_1.txt
rm -f tmp_network_tool_2.txt
rm -f tmp_base_adder.txt
rm -f tmp_base_network.txt
rm -f tmp_adder_network.txt

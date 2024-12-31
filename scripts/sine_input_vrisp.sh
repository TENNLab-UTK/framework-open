#!/bin/sh

if [ $# -ne 1 -a $# -ne 2 ]; then
  echo 'usage: sh scripts/sine_input.sh value(0 to 2pi) [network]' >&2
  echo 'This emits processor tool commands to calculate sine(value) neuromorphically' >&2
  exit 1
fi

x=$1
if [ $# = 2 ]; then echo ML $2 ; fi

# Get the "bin" -- number from 0 to 29.  Break 0 to 2pi into 30 "bins" and calculate
# the correct one.  Truncate the decimal.

bin=`echo $x | awk '{ v=$1*15/3.1415927; if (v < 0) { v = 0 }; if (v > 30) { v = 29}; print v}'`

# Pin the output if it's too big or too small:


# For bins 0 through 14, you'll spike b+1 on neuron 1, and 15-b on neuron 0
# For bins 15 through 29, you'll spike 30-b on neuron 1, and b+1-15 on neuron 0

if [ $(echo "$bin/1" | bc) -lt 15 ]; then
  s1=$(echo "(($bin*2)/1+1)" | bc)
  s0=$(echo "30-($bin*2/1)" | bc)
  s2=0
else
  s0=0
  s1=$(echo "(((30-$bin))*2+1)/1" | bc)
  s2=$(echo "(($bin-15)*2+1)/1" | bc)
fi

# Calculate what the answer should be:

sinx=`echo "" | awk '{ print sin('$x') }'`
nspikes=`echo $sinx | awk '{ printf "%.0f\n", ($1+1.5)/3*30 }'`

# Generate the spikes.  These are spiked in every three timesteps.

echo "CA"

for i in 0 1 2 ; do
  if [ $i = 0 ]; then ns=$s0 ; fi
  if [ $i = 1 ]; then ns=$s1 ; fi
  if [ $i = 2 ]; then ns=$s2 ; fi
  echo $i $ns | awk '{ for (i = 0; i < $2; i++) print "AS", $1, i, 1 }'
done

echo "RUN 60"
echo "OC"
echo ""
echo "# Value is "$x
echo "# sin(x) is "$sinx
echo "# Spikes on input neuron 0: "$s0
echo "# Spikes on input neuron 1: "$s1
echo "# Spikes on input neuron 2: "$s2
echo "# Number of output spikes should be: "$nspikes
echo ""


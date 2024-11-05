#!/bin/sh

if [ $# -ne 1 -a $# -ne 2 ]; then
  echo 'usage: sh scripts/sine_input.sh value(0 to 2pi) [network]' >&2
  echo 'This emits processor tool commands to calculate sine(value) neuromorphically' >&2
  exit 1
fi

x=$1
if [ $# = 2 ]; then echo ML $2 ; fi

# Get the "bin" -- number from 0 to 119.  Break 0 to 2pi into 120 "bins" and calculate
# the correct one.  Truncate the decimal.

bin=`echo $x | awk '{ v=$1*60/3.1415927; print v}' | sed 's/\..*//'`

# Pin the output if it's too big or too small:

if [ $bin -lt 0 ]; then bin=0; fi
if [ $bin -ge 120 ]; then bin=119; fi

# For bins 0 through 59, you'll spike b+1 on neuron 1, and 60-b on neuron 0
# For bins 60 through 119, you'll spike 120-b on neuron 1, and b+1-60 on neuron 0

if [ $bin -lt 60 ]; then
  s0=$((60-$bin))
  s1=$(($bin+1))
  s2=0
else
  s0=0
  s1=$((120-$bin))
  s2=$(($bin+1-60))
fi

# Calculate what the answer should be:

sinx=`echo "" | awk '{ print sin('$x') }'`
nspikes=`echo $sinx | awk '{ printf "%.0f\n", ($1+1.5)/3*120 }'`

# Generate the spikes.  These are spiked in every three timesteps.

echo "CA"

for i in 0 1 2 ; do
  if [ $i = 0 ]; then ns=$s0 ; fi
  if [ $i = 1 ]; then ns=$s1 ; fi
  if [ $i = 2 ]; then ns=$s2 ; fi
  echo $i $ns | awk '{ for (i = 0; i < $2; i++) print "AS", $1, i*3, 1 }'
done

echo "RUN 240"
echo "OC"
echo ""
echo "# Value is "$x
echo "# sin(x) is "$sinx
echo "# Spikes on input neuron 0: "$s0
echo "# Spikes on input neuron 1: "$s1
echo "# Spikes on input neuron 2: "$s2
echo "# Number of output spikes should be: "$nspikes
echo ""


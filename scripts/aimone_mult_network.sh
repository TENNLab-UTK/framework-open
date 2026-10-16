# This multiplies a number, spiked in as little-endian binary, by a constant that is baked
# into the SNN.   Eventually, I will make two major mods to this, but not now:
#
# 1. Instead of having this be a linear cascade of adders, I can make it tree, and
#    that will cut down on some timesteps.  Not too many (log w to log log w), but
#    it would take, say, 32 down to 5.
#
# 2. I'll tweak it to go general purpose multiplication.  This will take a minute.
#

if [ $# -ne 3 ]; then
  echo 'usage: sh scripts/aimone_adder.sh const w os_framework' >&2
  exit 1
fi

const=$1
w=$2
fro=$3

# Minor error checking.

if [ $const = 0 ]; then
  echo 'const cannot be 0' >&2
  exit 1
fi

# Compile necesssary programs in the open-source framework

for i in network_tool processor_tool_risp compose_networks ; do
  if [ ! -x $fro/bin/$i ]; then ( cd $fro ; make bin/$i ) fi
done

g++ junk.cpp


# Make an adder network

sh $fro/scripts/adder_network.sh 0 $fro > tmp_adder.txt

# Make an empty network with the proper RISP parameters.
# Do this after making the adder, because the adder script will delete it.

cat $fro/params/risp_127.txt | sed '/leak_mode/s/none/all/' > tmp_risp.txt
( echo M risp tmp_risp.txt
  echo EMPTYNET tmp_emptynet.txt ) | $fro/bin/processor_tool_risp

./a.out $const $w $fro | sh

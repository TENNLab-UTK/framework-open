# This creates a network that spikes w times.
#
# James S. Plank.  October, 2025.
#
# The network is printed on standard output, and information for running is
# put into tmp_info.txt.  

if [ $# -ne 2 ]; then
  echo 'usage: sh scripts/constant_network.sh w os_framework' >&2
  exit 1
fi

w=$1
fro=$2

# Compile necesssary programs in the open-source framework

for i in network_tool processor_tool_risp ; do
  if [ ! -x $fro/bin/$i ]; then ( cd $fro ; make bin/$i ) fi
done

# Make an empty network with the proper RISP parameters.

cat $fro/params/risp_127.txt | sed '/leak_mode/s/none/all/' > tmp_risp.txt

( echo M risp tmp_risp.txt
  echo EMPTYNET tmp_emptynet.txt ) | $fro/bin/processor_tool_risp

# Make the network

( echo FJ tmp_emptynet.txt
  echo AN 0 1 2
  echo AI 0 
  echo AO 2
  echo SETNAME 0 S
  echo SETNAME 1 K
  echo SETNAME 2 O

  echo AE 0 1   0 2   2 2   1 2
  echo SNP_ALL Threshold 1
  echo SEP_ALL Weight 1
  echo SEP_ALL Delay 1
  echo SEP 0 1 Delay $w
  echo SEP 1 2 Weight -1

  echo SORT Q
  echo TJ
  ) > tmp_network_tool.txt

$fro/bin/network_tool < tmp_network_tool.txt

( echo INPUT  S   0 Spike 1    0 
  echo OUTPUT O   2 TC_LE $w   1
  echo RUN $(($w+1))
) > tmp_info.txt

# rm -f tmp_risp.txt
# rm -f tmp_emptynet.txt
# rm -f tmp_network_tool.txt

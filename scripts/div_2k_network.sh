# This creates a network that divides a number, that is fed in a binary spike train
# in little endian, by 2^k.  K is a constant that is part of the network.
# It works with positive and negative numbers, where the negative
# numbers are represented by two's complement.
#
# w is used to isolate the output to w bits.  This is necessary, because sometimes there
# is overflow, and sometimes there is not overflow, and it is inconsistent (when you use
# two's complement for negative numbers).
#
# James S. Plank.  September, 2025.
#
# The network is printed on standard output, and information for running is
# put into tmp_info.txt.  

if [ $# -ne 3 ]; then
  echo 'usage: sh scripts/div_2k_network.sh k w os_framework' >&2
  exit 1
fi

k=$1
w=$2
fro=$3

# Compile necesssary programs in the open-source framework

for i in network_tool processor_tool_risp ; do
  if [ ! -x $fro/bin/$i ]; then ( cd $fro ; make bin/$i ) fi
done

# Make an empty network with the proper RISP parameters.

cat $fro/params/risp_127.txt | sed '/leak_mode/s/none/all/' > tmp_risp.txt

( echo M risp tmp_risp.txt
  echo EMPTYNET tmp_emptynet.txt ) | $fro/bin/processor_tool_risp

# Create the network and put print it on standard output.

i=0
a=$i ; i=$(($i+1))
s=$i ; i=$(($i+1))
c=$i ; i=$(($i+1))
b=$i ; i=$(($i+1))
sf1=$i ; i=$(($i+1))
sf2=$i ; i=$(($i+1))
of=$i ; i=$(($i+1))
bs=$i ; i=$(($i+1))
ss1=$i ; i=$(($i+1))
ss2=$i ; i=$(($i+1))
os=$i ; i=$(($i+1))
sm=$i ; i=$(($i+1))
cm=$i ; i=$(($i+1))
bm=$i ; i=$(($i+1))
sp=$i ; i=$(($i+1))
cp=$i ; i=$(($i+1))
bp=$i ; i=$(($i+1))
om=$i ; i=$(($i+1))
op=$i ; i=$(($i+1))
q=$i ; i=$(($i+1))

( echo FJ tmp_emptynet.txt
  echo AN 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19
  echo AI $a $s
  echo AO $q
  echo SETNAME $a V0
  echo SETNAME $s S
  echo SETNAME $c C
  echo SETNAME $b B
  echo SETNAME $sf1 SF1
  echo SETNAME $sf2 SF2
  echo SETNAME $of OF
  echo SETNAME $bs BS
  echo SETNAME $ss1 SS1
  echo SETNAME $ss2 SS2
  echo SETNAME $os OS
  echo SETNAME $sm S-
  echo SETNAME $cm C-
  echo SETNAME $bm B-
  echo SETNAME $sp S+
  echo SETNAME $cp C+
  echo SETNAME $bp B+
  echo SETNAME $om O-
  echo SETNAME $op O+
  echo SETNAME $q Q
  echo SNP_ALL Threshold 1
  echo SNP $sf2 $bs $ss2 $sm $om $op Threshold 2
  
  echo AE $a $sf1   $a $sf2   $a $sm   $a $sp   $a $op
  echo AE $s $c   $s $b   $s $sf1   $s $sf2   $s $bs   $s $ss1   $s $ss2   $s $sm   $s  $sp
  echo AE $c $b
  echo AE $b $b    $b $sf1   $b $sf2   $b $bs   $b $ss1   $b $ss2
  echo AE $sf1 $of             
  echo AE $sf2 $sf1   $sf2 $sf2   $sf2 $of
  echo AE $of $bs
  echo AE $bs $ss1   $bs $ss2
  echo AE $ss1 $os
  echo AE $ss2 $ss1   $ss2 $ss2   $ss2 $os
  echo AE $os $om
  echo AE $sm $cm   $sm $bm
  echo AE $cm $bm
  echo AE $bm $bm   $bm $om
  echo AE $sp $cp   $sp $bp
  echo AE $cp $bp
  echo AE $bp $bp   $bp $op
  echo AE $om $q
  echo AE $op $q

  echo SEP_ALL Weight 1
  echo SEP_ALL Delay 1

  echo SEP $a $sf1   $a $sf2   $a $sp                    Weight -1
  echo SEP $a $op                                        Delay $(($w-$k+5))

  echo SEP $s $c                                         Delay $(($w-1))
  echo SEP $s $sm   $s $sp                               Delay $w
  echo SEP $s $bs                                        Delay $(($k+3))
  echo SEP $s $ss1   $s $ss2                             Delay $(($k+4))
  echo SEP $s $sf1   $s $sf2   $s $ss1   $s $ss2         Weight 2

  echo SEP $c $b                                         Weight -1

  echo SEP $b $bs                                        Delay $(($k+3))
  echo SEP $b $ss1   $b $ss2                             Delay $(($k+4))

  echo SEP $sf2   $of                                    Weight -1

  echo SEP $bs $ss1   $bs $ss2                           Weight -1

  echo SEP $ss2   $os                                    Weight -1

  echo SEP $os   $om                                     Delay $(($w-$k))

  echo SEP $sm $cm                                       Delay $w
  echo SEP $cm $bm                                       Weight -1
  echo SEP $bm $om                                       Delay 4

  echo SEP $sp $cp                                       Delay $w
  echo SEP $cp $bp                                       Weight -1
  echo SEP $bp $op                                       Delay 4

  echo SORT Q
  echo TJ
  ) > tmp_network_tool.txt

$fro/bin/network_tool < tmp_network_tool.txt

# Info is the form:
#
# INPUT  node_name node_number format timesteps starting_timestep
# OUTPUT node_name node_number format timesteps starting_timestep
# RUN timesteps

( echo INPUT  V0  $a TC_LE $w   0 
  echo INPUT  S   $s Spike 1    0 
  echo OUTPUT QUOTIENT $q TC_LE $w $(($w+6)) 
  echo RUN $(($w+$w+6))
) > tmp_info.txt

rm -f tmp_risp.txt
rm -f tmp_emptynet.txt
rm -f tmp_network_tool.txt

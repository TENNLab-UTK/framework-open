if [ $# -ne 2 ]; then
  echo 'usage: sh scripts/diff_network.sh max_val encoder(t|s|v)' >&2
  exit 1
fi

min=0
max=$1
enc=$2

if [ "$enc" != s -a "$enc" != t -a "$enc" != v ]; then
  echo 'Third argument must be t, s or v' >&2
  exit 1
fi

if [ "$enc" == s ]; then echo 'Have not implemented encoder=s yet' >*2 ; exit 1 ; fi
if [ "$enc" == v ]; then echo 'Have not implemented encoder=v yet' >*2 ; exit 1 ; fi

fro=.

if [ ! -x $fro/bin/processor_tool_risp ]; then
  echo "You need to make $fro/bin/processor_tool_risp" >&2
  exit 1
fi

if [ ! -x $fro/bin/network_tool ]; then
  echo "You need to make $fro/bin/network_tool" >&2
  exit 1
fi

if [ ! -f $fro/params/risp_127.txt ]; then
  echo no file $fro/params/risp_127.txt >&2
  exit 1
fi

maxdelay=$max
maxthresh=$max

sed -e "s/127/$maxthresh/" \
    -e '/max_delay/s/[0-9][0-9]*/'$maxdelay'/' $fro/params/risp_127.txt > tmp_params.txt

( echo M risp tmp_params.txt ; echo EMPTYNET tmp_net.txt ) |
    $fro/bin/processor_tool_risp 2>/dev/null

n=0
( echo FJ tmp_net.txt

  # Create the two input nodes, and the output node

  echo AN $n
  echo AI $n
  echo SNP $n Threshold 1
  echo SETNAME $n 'I1'
  i1=$n
  n=$(($n+1))

  echo AN $n
  echo AI $n
  echo SNP $n Threshold 1
  echo SETNAME $n 'I2'
  i2=$n
  n=$(($n+1))

  echo AN $n
  echo AI $n
  echo SNP $n Threshold 1
  echo SETNAME $n 'S'
  s=$n
  n=$(($n+1))

  echo AN $n
  echo SNP $n Threshold 1
  echo SETNAME $n 'O'
  o=$n
  n=$(($n+1))

  # Create the two nodes for diffs.

  echo AN $n
  echo SNP $n Threshold 1
  echo SETNAME $n 'D1'
  d1=$n
  n=$(($n+1))

  echo AN $n
  echo SNP $n Threshold 1
  echo SETNAME $n 'D2'
  d2=$n
  n=$(($n+1))

  # Create A2 and S2

  echo AN $n
  echo AO $n
  echo SNP $n Threshold $max
  echo SETNAME $n 'A2'
  a2=$n
  n=$(($n+1))

  echo AN $n
  echo SNP $n Threshold 1
  echo SETNAME $n 'S2'
  s2=$n
  n=$(($n+1))

  # Edges from the two input nodes to the diff nodes, plus the self edges.

  echo AE $i1 $d1
  echo SEP $i1 $d1 Weight 1
  echo SEP $i1 $d1 Delay 1

  echo AE $i1 $d2
  echo SEP $i1 $d2 Weight -1
  echo SEP $i1 $d2 Delay 1

  echo AE $i2 $d1
  echo SEP $i2 $d1 Weight -1
  echo SEP $i2 $d1 Delay 1

  echo AE $i2 $d2
  echo SEP $i2 $d2 Weight 1
  echo SEP $i2 $d2 Delay 1

  echo AE $d1 $d1
  echo SEP $d1 $d1 Weight 1
  echo SEP $d1 $d1 Delay 1

  echo AE $d2 $d2
  echo SEP $d2 $d2 Weight 1
  echo SEP $d2 $d2 Delay 1

  # Edges to A2 

  echo AE $d1 $a2
  echo SEP $d1 $a2 Weight -1
  echo SEP $d1 $a2 Delay 1

  echo AE $d2 $a2
  echo SEP $d2 $a2 Weight 1
  echo SEP $d2 $a2 Delay 1

  echo AE $a2 $a2
  echo SEP $a2 $a2 Weight -1
  echo SEP $a2 $a2 Delay 1

  # Edges involving S2

  echo AE $s $s2
  echo SEP $s $s2 Weight 1
  echo SEP $s $s2 Delay $max

  echo AE $s2 $s2
  echo SEP $s2 $s2 Weight 1
  echo SEP $s2 $s2 Delay 1

  echo AE $s2 $a2
  echo SEP $s2 $a2 Weight 1
  echo SEP $s2 $a2 Delay 1

  echo AE $a2 $s2
  echo SEP $a2 $s2 Weight -1
  echo SEP $a2 $s2 Delay 1

  echo SORT Q
  echo TJ 
) | $fro/bin/network_tool

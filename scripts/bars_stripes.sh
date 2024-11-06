if [ $# -ne 2 ]; then
  echo 'usage: sh scripts/bars_stripes.sh rows cols' >&2
  exit 1
fi

r=$1
c=$2
fro=.

if [ ! -f $fro/params/risp_1_plus.txt ]; then
  echo no file $fro/params/risp_1_plus.txt >&2
  exit 1
fi

if [ $r -ge $c ]; then mv=$r ; else mv=$c ; fi

sed "/max_threshold/s/1/$mv/" $fro/params/risp_1_plus.txt  |
     sed '/max_delay/s/15/1/' |
     sed '/leak_mode/s/none/all/' > tmp_params.txt

( echo M risp tmp_params.txt ; echo EMPTYNET tmp_net.txt ) | $fro/bin/processor_tool_risp 2>/dev/null

n=0
( echo FJ tmp_net.txt

  # Create the output nodes first.

  echo AN $n
  echo AO $n
  echo SNP $n Threshold 1
  echo SETNAME $n 'O-Bar'
  bar=$n
  n=$(($n+1))

  echo AN $n
  echo AO $n
  echo SNP $n Threshold 1
  echo SETNAME $n 'O-Stripe'
  stripe=$n
  n=$(($n+1))

  # Hidden nodes for stripes, and their synapses.  The params guarantee that every edge has
  # a weight of 1 and a delay of 1.  That's nice, although I could use SEP_ALL as well.

  bstart=$n
  i=0
  while [ $i -lt $r ]; do
    echo AN $n
    echo SNP $n Threshold $c
    echo SETNAME $n 'H-S'$i
    echo AE $n $stripe
    n=$(($n+1))
    i=$(($i+1))
  done
    
  # Hidden nodes for stripes

  sstart=$n
  j=0
  while [ $j -lt $c ]; do
    echo AN $n
    echo SNP $n Threshold $r
    echo SETNAME $n 'H-B'$j
    echo AE $n $bar
    n=$(($n+1))
    j=$(($j+1))
  done
    
  # Now the inputs

  istart=$n
  i=0
  while [ $i -lt $r ]; do
    j=0
    while [ $j -lt $c ]; do
      echo AN $n
      echo AI $n
      echo SNP $n Threshold 1
      echo SETNAME $n 'I['$i']['$j']'
      echo AE $n $(($bstart+$i))
      echo AE $n $(($sstart+$j))

      n=$(($n+1))
      j=$(($j+1))
    done
    i=$(($i+1))
  done
  echo SORT Q
  echo TJ 
) | $fro/bin/network_tool

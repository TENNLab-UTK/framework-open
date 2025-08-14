# This creates a diff network using scripts/diff_network.sh, then
# runs it to see how it does.

if [ $# -ne 3 ]; then
  echo 'usage: sh scripts/diff_run.sh v1 v2 risp-n - -1 <= v1,v2 <= 1, e.g. 127 for RISP-127' >&2
  exit 1
fi

if [ ! -x bin/processor_tool_risp ]; then
  echo 'no program bin/processor_tool_risp' >&2
  exit 1
fi

if [ ! -x bin/network_tool ]; then
  echo 'no program bin/network_tool' >&2
  exit 1
fi

if [ ! -f scripts/diff_network.sh ]; then
  echo 'no file scripts/diff_network.sh' >&2
  exit 1
fi

v1=`echo $1 | awk '{ printf "%.0f\n", (1-$1)/2.0*'$3' }'`
v2=`echo $2 | awk '{ printf "%.0f\n", (1-$1)/2.0*'$3' }'`
max=$3

sh scripts/diff_network.sh $3 t > tmp.txt

time=`( echo ML tmp.txt
  echo AS 0 $v1 1
  echo AS 1 $v2 1
  echo AS 2 0 1
  echo RUN $(($max+$max+10+$max))
  echo OT ) | bin/processor_tool_risp | awk '{ printf "%d\n", $NF }'`

# -1 minus 1 = -2, which will be at time $max+1
# 1 minus -1 = 2, which will be at time $max*3.
# So, interpolate the time between these values to get the answer.

if [ $time = $((max+1)) ]; then 
  diff=-2
else
  diff=`echo $max $time | awk '{ print ($2-($1))*4/(2*$1)-2 }'`
fi

echo $1 minus $2 using RISP-$max.  Spike times: $v1 $v2.  Output time: $time.  Answer: $diff

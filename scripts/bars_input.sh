if [ $# -ne 4 ]; then
  echo 'usage: sh scripts/bs_input.sh network rows cols starting_neuron < pixels' >&2
  exit 1
fi

network=$1
rows=$2
cols=$3
sn=$4

input=""
nr=0

while read a ; do
  nr=$(($nr+1))
  if [ $nr -gt $rows ]; then
    echo Too many rows. >&2
    exit 1
  fi

  if [ `echo $a | sed -e 's/1/0/g' -e 's/0*/0/'` != 0 ]; then
    echo $a has at least one non-zero/non-one character >&2
    exit 1
  fi
  c=`echo $a | wc | awk '{ print $3-1 }'`
  if [ $c != $cols ]; then
    echo $a - not $cols columns >&2
    exit 1
  fi
  if [ a"$input" != a ]; then input="$input "; fi
  input="$input$a"
done

echo ML $1
for i in $input ; do
  echo $i | sed 's/\(.\)/\1 /g' |
    awk '{ for (i = 1; i <= NF; i++) if ($i == 1) print "AS", i-1+'$sn', 0, 1 }'
  sn=$(($sn+$cols))
done
echo RUN 3
echo OT

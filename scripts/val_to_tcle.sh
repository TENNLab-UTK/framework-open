# Converts a double to binary of two's complement little endian with w bits.
# This feels like a really dumb shell script, but there you have it.

if [ $# -ne 2 ]; then
  echo 'usage: sh val_to_tcle.sh val w' >&2
  exit 1
fi

v=$1
w=$2

# Turn into an int

v=`echo $v | awk '{ printf "%.0f\n", $1 }'`

top=1
for i in `seq $w` ; do top=$(($top*2)); done
if [ $v -ge $top ]; then
  echo "Value $v is too big -- must be < $top" >&2
  exit 1
fi

if [ $v -le -$top ]; then
  echo "Value $v is too small -- must be > -$top" >&2
  exit 1
fi

# If v is positive, then turn it into le:

if [ $v -ge 0 ]; then
  sr=""
  for i in `seq $w` ; do
    b=$(($v%2))
    v=$(($v/2))
    sr="$sr"$b
  done
  echo $sr
else

  sr=""
  nv=$((-$v))
  c=1
  for i in `seq $w` ; do
    b=$((1-$nv%2))
    nv=$(($nv/2))
    if [ $c = 1 -a $b = 1 ]; then
      sr="$sr"0
    elif [ $c = 1 ]; then
      sr="$sr"1
      c=0
    else
      sr="$sr"$b
      c=0
    fi
  done
  echo $sr
fi

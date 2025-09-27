# Converts a double to binary of two's complement little endian with w bits.
# This feels like a really dumb shell script, but there you have it.

if [ $# -ne 1 ]; then
  echo 'usage: sh tcle_to_val.sh spike-raster' >&2
  exit 1
fi

sr=$1

if [ `echo $sr | sed 's/0/1/g' | sed 's/1*/1/'` != 1 ]; then
  echo "Bad spike raster $sr - must be composed only of zeros and ones" >&2
  exit 1
fi

ld=`echo $sr | sed 's/.*\(.\)$/\1/'`
if [ $ld = 1 ]; then
  sr=`echo $sr | sed 's/0/A/g' | sed 's/1/0/g' | sed 's/A/1/g'`
fi

v=0
d=1
tsr=$sr
while [ a"$tsr" != a ]; do
  fd=`echo $tsr | sed 's/\(.\).*/\1/'`
  tsr=`echo $tsr | sed 's/.//'`
  v=$(($v+($fd*$d)))
  d=$(($d*2))
done

if [ $ld = 1 ]; then v=$((-($v+1))) ; fi

echo $v
  

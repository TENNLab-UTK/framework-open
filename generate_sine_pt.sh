#!/usr/bin/env sh
pi=3.141592653589793

echo "ML tmp_network.txt"

for i in {0..239} ; do
    val=$(echo "scale=30; ($pi*2)*($i/240)/1" | bc)
    sh scripts/sine_input_vrisp.sh $(printf "%7f" $val)
done

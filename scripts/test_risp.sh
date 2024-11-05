# Script to test the network tool and RISP processor tool

if [ $# -ne 2 ]; then
  echo 'usage: sh scripts/test_risp.sh number('-' for all) yes|no(keep temporary files)' >&2
  exit 1
fi

keep="$2"
if [ "$keep" != yes -a "$keep" != no ]; then
  echo 'keep parameter must be "yes" or "no".' >&2
  exit 1
fi

t="$1"

if [ T"$t" = 'T-' ]; then
  t=`ls testing`
fi

# Make the two executables if they aren't made yet.

for i in bin/network_tool bin/processor_tool_risp ; do
  if [ ! -x $i ]; then make $i ; fi
done

for i in $t ; do
  i=`echo $i | awk '{ printf "%02d\n", $1 }'`
  if [ ! -d testing/$i ]; then
    echo "Error -- no directory testing/$i" >&2
    exit 1
  fi

  for f in label.txt network_tool.txt processor.sh processor_tool.txt correct_output.txt ; do
    if [ ! -f testing/$i/$f ]; then
      echo "Error -- no file testing/$i/$f" >&2
      exit 1
    fi
  done

  # Create the processor params, and then an empty network

  l=`cat testing/$i/label.txt`
  sh testing/$i/processor.sh > tmp_proc_params.txt
  
  ( echo M risp tmp_proc_params.txt
    echo EMPTYNET tmp_empty_network.txt ) | bin/processor_tool_risp

  # Use network_tool.txt to create the network.

  bin/network_tool < testing/$i/network_tool.txt > tmp_nt_output.txt 2>&1
  if [ `wc tmp_nt_output.txt | awk '{ print $1 }'` != 0 ]; then
    echo "Test $i - $l" >&2
    echo "There was an error in the network_tool command when I ran:" >&2
    echo "" >&2
    echo "bin/network_tool < testing/$i/network_tool.txt > tmp_nt_output.txt" >&2
    echo "" >&2
    cat tmp_nt_output.txt >&2
    exit 1
  fi

  # Now, you'll execute the commands in processor_tool.txt and compare the output
  # against known output.

  cp testing/$i/processor_tool.txt tmp_pt_input.txt

  bin/processor_tool_risp < testing/$i/processor_tool.txt > tmp_pt_output.txt 2> tmp_pt_error.txt
  if [ `wc tmp_pt_error.txt | awk '{ print $1 }'` != 0 ]; then
    echo "Test $i - $l" >&2
    echo "There was an error in the processor_tool_risp command when I ran:" >&2
    echo "" >&2
    echo "bin/processor_tool_risp < testing/$i/processor_tool.txt" >&2
    echo "" >&2
    cat tmp_pt_error.txt >&2
    exit 1
  fi

  d=`diff tmp_pt_output.txt testing/$i/correct_output.txt | wc | awk '{ print $1 }'`
  if [ $d != 0 ]; then
    echo "Test $i - $l" >&2
    echo "Error: Output does not match the correct output." >&2
    echo "       Output file is tmp_pt_output.txt" >&2
    echo "       Correct output file is testing/$i/correct_output.txt" >&2
    exit 1
  fi
    
  echo "Passed Test $i - $l"
  if [ $keep = no ]; then
    rm -f tmp_proc_params.txt \
          tmp_network.txt \
          tmp_nt_output.txt \
          tmp_pt_output.txt \
          tmp_pt_input.txt \
          tmp_pt_error.txt \
          tmp_empty_network.txt
  fi

done

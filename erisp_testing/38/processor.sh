cat params/risp_1_plus.txt |
sed "/max_threshold/s/1/7/" |
     sed '/max_delay/s/15/1/' |
     sed '/leak_mode/s/none/all/' |
     sed '/max_delay/s/,/, "spike_value_factor": 1,/'

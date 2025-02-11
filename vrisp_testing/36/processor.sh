cat params/vrisp_1_plus.json |
sed "/max_threshold/s/1/7/" |
     sed '/max_delay/s/15/1/' |
     sed '/tracked_timesteps/s/16/2/' |
     sed '/leak_mode/s/none/all/' |
     sed '/max_delay/s/,/, "spike_value_factor": 1,/'

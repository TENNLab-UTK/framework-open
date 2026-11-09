sed -e '/min_weight/s/$/ "noisy_stddev": 0.01, "noisy_seed": 1,/' -e 's/generative/legacy/' params/risp_f.txt

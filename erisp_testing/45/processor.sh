cat params/risp_127.txt | jq '.nonpositive_threshold_mode = "legacy" | .min_threshold = 0.0'

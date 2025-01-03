#!/usr/bin/env sh
set -euo pipefail

if [[ $# -ne 5 ]]; then
    echo "usage: $0 processor_name params.json input_rows input_cols downsample_rate"
    exit 1
fi

processor_name=$1
params=$2
input_rows=$3
input_cols=$4
downsample_rate=$5

input_size=$((input_rows*input_cols))
downsampled_cols=$((input_cols/(downsample_rate/2)))

temp_file=$(mktemp)
trap 'rm -f $temp_file' 0 2 3 15

(echo "M $processor_name $params"; echo "EMPTYNET") | "bin/processor_tool_$processor_name" > "$temp_file"

(
    echo "fj $temp_file";
    printf "AN ";
    for ((i=0; i < input_rows*input_cols + (input_rows*input_cols/downsample_rate); i++)); do
        printf '%s ' "$i";
    done;
    echo;
    printf "AI ";
    for ((i=0; i < input_rows*input_cols; i++)); do
        printf '%s ' "$i";
    done;
    echo;
    printf "AO ";
    for ((i=0; i < (input_rows*input_cols/downsample_rate); i++)); do
        printf '%d ' "$((i+(input_rows*input_cols)))";
    done
    echo;
    echo "SNP_ALL Threshold 1";
    for ((i=0; i < (input_rows*input_cols/downsample_rate); i++)); do
        printf 'SNP %d Threshold %d\n' "$((i+(input_rows*input_cols)))" "$((downsample_rate/2))";
    done
    for ((i=0; i < input_rows; i++)); do
        for ((j=0; j < input_cols; j++)); do
            current_node=$((i*input_cols + j))
            downsampled_row=$((i/(downsample_rate/2)))
            downsampled_col=$((j/(downsample_rate/2)))
            target_node=$((downsampled_row*downsampled_cols+downsampled_col + input_size))
            echo "AE $current_node $target_node"
        done;
    done;
    echo "SEP_ALL Weight 1";
    echo "SEP_ALL Delay 1";
    echo "TJ";
) | bin/network_tool

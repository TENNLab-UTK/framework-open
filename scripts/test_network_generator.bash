#!/usr/bin/env bash

set -euo pipefail

main() {
    if [[ $# -ne 5 ]]; then
        echo "usage: $0 processor_name params.json input_neurons output_neurons connectivity_chance"
        exit 1
    fi

    local processor_name="${1}"
    local params="${2}"
    local input_neurons="${3}"
    local output_neurons="${4}"
    local connectivity_chance="${5}"

    local total_neurons=$((input_neurons + output_neurons))

    tmp_file=$(mktemp)
    trap 'rm -f "${tmp_file}"' 0 2 3 15

    {
        echo "M ${processor_name} ${params}"
        echo "EMPTYNET"
    } | "bin/processor_tool_${processor_name}" >"${tmp_file}"

    {
        echo "FJ $tmp_file"

        printf "AN "
        for ((i = 0; i < total_neurons; i++)); do
            printf '%s ' "${i}"
        done
        echo

        printf "AI "
        for ((i = 0; i < input_neurons; i++)); do
            printf '%s ' "${i}"
        done
        echo

        printf "AO "
        for ((i = 0; i < output_neurons; i++)); do
            printf '%s ' "$((i + input_neurons))"
        done
        echo

        echo "SNP_ALL Threshold 1"

        for ((i = 0; i < input_neurons; i++)); do
            for ((j = 0; j < output_neurons; j++)); do
                if [[ $((RANDOM % connectivity_chance)) -eq 0 ]]; then
                    printf 'AE %s %s\n' "${i}" "$((j + input_neurons))"
                fi
            done
        done

        echo "SEP_ALL Weight 1"
        echo "SEP_ALL Delay 1"
        echo "SORT Q"
        echo "TJ"
    } | bin/network_tool
}

main "${@}"

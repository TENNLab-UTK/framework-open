#!/usr/bin/env bash

main() {
    if [[ $# -ne 1 ]]; then
        echo "usage: $0 connectivity_chance"
        exit 1
    fi

    local connectivity_chance="${1}"

    local vr_fired=($(
        bin/downsample_app_vrisp_vector connectivity_testing/vrisp_"${connectivity_chance}".json 0 | tail -1 | awk -F ':' '{ print 1/$2 }'
        for i in {10..1..-1}; do
            bin/downsample_app_vrisp_vector connectivity_testing/vrisp_"${connectivity_chance}".json "${i}" | tail -1 | awk -F ':' '{ print 1/$2 }'
        done
    ))
    local vr_synapses=($(
        bin/downsample_app_vrisp_vector_synapse connectivity_testing/vrisp_"${connectivity_chance}".json 0 | tail -1 | awk -F ':' '{ print 1/$2 }'
        for i in {10..1..-1}; do
            bin/downsample_app_vrisp_vector_synapse connectivity_testing/vrisp_"${connectivity_chance}".json "${i}" | tail -1 | awk -F ':' '{ print 1/$2 }'
        done
    ))
    local vrisp=($(
        bin/downsample_app_vrisp connectivity_testing/vrisp_"${connectivity_chance}".json 0 | tail -1 | awk -F ':' '{ print 1/$2 }'
        for i in {10..1..-1}; do
            bin/downsample_app_vrisp connectivity_testing/vrisp_"${connectivity_chance}".json "${i}" | tail -1 | awk -F ':' '{ print 1/$2 }'
        done
    ))
    local risp=($(
        bin/downsample_app_risp connectivity_testing/risp_"${connectivity_chance}".json 0 | tail -1 | awk -F ':' '{ print 1/$2 }'
        for i in {10..1..-1}; do
            bin/downsample_app_risp connectivity_testing/risp_"${connectivity_chance}".json "${i}" | tail -1 | awk -F ':' '{ print 1/$2 }'
        done
    ))

    echo "${vr_fired[@]}"
    echo "${vr_synapses[@]}"
    echo "${vrisp[@]}"
    echo "${risp[@]}"

    printf '| | vrisp vector - fired | vrisp vector - synapses | vrisp | risp |\n'
    printf '| 0%% | %s | %s | %s | %s |\n' "${vr_fired[0]}" "${vr_synapses[0]}" "${vrisp[0]}" "${risp[0]}"
    for i in {1..10}; do
        printf '| %.1f%% | %s | %s | %s | %s |\n' "$(echo 1/"$((10 - i + 1))"*100 | bc -l)" "${vr_fired[$i]}" "${vr_synapses[$i]}" "${vrisp[$i]}" "${risp[$i]}"
    done

}

main "${@}"

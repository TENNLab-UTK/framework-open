#!/usr/bin/env bash

main() {
    if [ $# -ne 1 ]; then
        echo "usage: $0 network.json"
        exit 1
    fi
    local network="${1}"

    echo "Average Fan-out:" "$(jq '(.Edges | length) / (.Nodes | length)' "${network}")"
}

main "${@}"

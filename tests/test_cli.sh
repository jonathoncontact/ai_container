#!/bin/sh
set -eu

if [ "$#" -ne 1 ]; then
    echo "usage: $0 path-to-app" >&2
    exit 2
fi

app_path=$1
output=$(printf '%s\n' \
    'h' \
    'r 0 1' \
    'f 1 1 extra' \
    'f 1 1' \
    'r 1 1' \
    'f 1 1' \
    'r 1 1' \
    'q' | "$app_path")

printf '%s\n' "$output" | grep -F 'Minesweeper' >/dev/null
printf '%s\n' "$output" | grep -F 'Rows and columns use one-based coordinates' >/dev/null
printf '%s\n' "$output" | grep -F 'Error: use r <row> <column>' >/dev/null
printf '%s\n' "$output" | grep -F 'Error: use f <row> <column>' >/dev/null
printf '%s\n' "$output" | grep -F 'Cell flagged.' >/dev/null
printf '%s\n' "$output" | grep -F 'Unflag the cell before revealing it.' >/dev/null
printf '%s\n' "$output" | grep -F 'Flag removed.' >/dev/null
printf '%s\n' "$output" | grep -F 'Cell revealed.' >/dev/null
printf '%s\n' "$output" | grep -F 'Goodbye.' >/dev/null

eof_output=$(printf '%s\n' 'h' | "$app_path")
printf '%s\n' "$eof_output" | grep -F 'Commands:' >/dev/null

printf '%s\n' 'CLI smoke tests passed.'

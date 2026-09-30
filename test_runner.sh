#!/bin/sh
set -eu

build_dir=build
app_path="$build_dir/minesweeper_app"
test_path="$build_dir/minesweeper_tests"
common_flags='-std=c++17 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion'

mkdir -p "$build_dir"

# shellcheck disable=SC2086
g++ $common_flags main.cpp minesweeper.cpp -o "$app_path"
# shellcheck disable=SC2086
g++ $common_flags minesweeper.cpp tests/minesweeper_tests.cpp -o "$test_path"

"$test_path"
./tests/test_cli.sh "$app_path"

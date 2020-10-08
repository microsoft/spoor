#!/usr/bin/env bash

set -eu

WORKSPACE="$(bazel info workspace)"

function run_clang_format {
  echo "Formatting $1"
  clang-format \
    -style=file \
    -i \
    "$1"
}

if [ $# -eq 0 ]; then
  find "$WORKSPACE" \
    -type f \
    \( -iname '*.h' -o -iname '*.cc' -o -iname '*.proto' ! -iname 'bazel-' \) \
    -print0 |
      while read -d $'\0' file_name; do
        run_clang_format "$file_name"
      done
else
  for file_name in "$@"; do
    run_clang_format "$file_name"
  done
fi

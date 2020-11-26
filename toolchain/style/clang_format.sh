#!/usr/bin/env bash
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set -eu

WORKSPACE="$(bazel info workspace)"

if command -v clang-format-11 &> /dev/null; then
  CLANG_FORMAT="clang-format-11"
elif command -v clang-format &> /dev/null; then
  CLANG_FORMAT="clang-format"
else
  echo "clang-format not found"
  exit 1
fi

function run_clang_format {
  echo "Formatting $1"
  "$CLANG_FORMAT" \
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

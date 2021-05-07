#!/usr/bin/env bash
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Usage:
# `clang_format.sh output_file input_file args...`

set -eu

if command -v clang-format-12 &> /dev/null; then
  CLANG_FORMAT="clang-format-12"
elif command -v clang-format &> /dev/null; then
  CLANG_FORMAT="clang-format"
else
  echo "clang-format not found"
  exit 1
fi

OUTPUT_FILE="$1"
shift
INPUT_FILE="$1"

"$CLANG_FORMAT" "$@" 
cp "$INPUT_FILE" "$OUTPUT_FILE"

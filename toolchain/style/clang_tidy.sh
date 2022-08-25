#!/usr/bin/env bash
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Usage:
# `clang_tidy.sh input_file --export-fixes output_file -- args...`

set -eu

if command -v clang-tidy-14 &> /dev/null; then
  CLANG_TIDY="clang-tidy-14"
elif command -v clang-tidy &> /dev/null; then
  CLANG_TIDY="clang-tidy"
else
  echo "clang-tidy not found"
  exit 1
fi

OUTPUT_FILE="$3"

# The clang_tidy Aspect requires an output file, however, Clang Tidy does not
# create an output file if there are no errors.
touch "$OUTPUT_FILE"

"$CLANG_TIDY" "$@"

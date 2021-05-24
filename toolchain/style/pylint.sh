#!/usr/bin/env bash
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Usage:
# `pylint.sh output_file input_file args...`

set -eu

if command -v pylint &> /dev/null; then
  PYLINT="pylint"
else
  echo "pylint not found"
  exit 1
fi

OUTPUT_FILE="$1"
shift
INPUT_FILE="$1"

"$PYLINT" "$@"
cp "$INPUT_FILE" "$OUTPUT_FILE"

#!/usr/bin/env bash
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Usage:
# `yapf.sh output_file input_file args...`

set -eu

if command -v yapf &> /dev/null; then
  YAPF="yapf"
else
  echo "yapf not found"
  exit 1
fi

OUTPUT_FILE="$1"
shift
INPUT_FILE="$1"

"$YAPF" "$@"
cp "$INPUT_FILE" "$OUTPUT_FILE"

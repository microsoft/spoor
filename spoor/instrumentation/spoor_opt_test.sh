#!/usr/bin/env bash
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set -e

BASE_PATH="spoor/instrumentation"
SPOOR_OPT="$BASE_PATH/spoor_opt"
OUTPUT_IR_FILE="instrumented.ll"

"$SPOOR_OPT" "$BASE_PATH/test_data/fib.ll" \
  --output_language=ir \
  --output_file="$OUTPUT_IR_FILE"

FUNCTION_MAP_FILE=$(find . -type f -name "*.spoor_function_map")

if ! [[ -s "$FUNCTION_MAP_FILE" ]]; then
  echo "The function map file '$FUNCTION_MAP_FILE' is empty."
  exit 1
fi

grep _spoor_runtime_LogFunctionEntry "$OUTPUT_IR_FILE" > /dev/null
grep _spoor_runtime_LogFunctionExit "$OUTPUT_IR_FILE" > /dev/null

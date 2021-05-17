#!/usr/bin/env bash
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set -e

BASE_PATH="spoor/instrumentation"
SPOOR_OPT="$BASE_PATH/spoor_opt"
OUTPUT_IR_FILE="fib_instrumented.ll"
OUTPUT_INSTRUMENTED_FUNCTION_MAP_FILE="fib.spoor_function_map"

"$SPOOR_OPT" "$BASE_PATH/test_data/fib.ll" \
  --output_language=ir \
  --output_function_map_file="$OUTPUT_INSTRUMENTED_FUNCTION_MAP_FILE" \
  --output_file="$OUTPUT_IR_FILE"

if ! [[ -s "$OUTPUT_INSTRUMENTED_FUNCTION_MAP_FILE" ]]; then
  echo "The function map file '$OUTPUT_INSTRUMENTED_FUNCTION_MAP_FILE' is" \
      "empty or was not created."
  exit 1
fi

grep _spoor_runtime_LogFunctionEntry "$OUTPUT_IR_FILE" > /dev/null
grep _spoor_runtime_LogFunctionExit "$OUTPUT_IR_FILE" > /dev/null

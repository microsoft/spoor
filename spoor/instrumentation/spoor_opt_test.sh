#!/usr/bin/env bash
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set -e

BASE_PATH="spoor/instrumentation"
SPOOR_OPT="$BASE_PATH/spoor_opt"
OUTPUT_IR_FILE="fib_instrumented.ll"
OUTPUT_INSTRUMENTED_SYMBOLS_FILE="fib.spoor_symbols"

"$SPOOR_OPT" "$BASE_PATH/test_data/fib.ll" \
  --output_language=ir \
  --output_symbols_file="$OUTPUT_INSTRUMENTED_SYMBOLS_FILE" \
  --output_file="$OUTPUT_IR_FILE"

if ! [[ -s "$OUTPUT_INSTRUMENTED_SYMBOLS_FILE" ]]; then
  echo "The function map file '$OUTPUT_INSTRUMENTED_SYMBOLS_FILE' is" \
      "empty or was not created."
  exit 1
fi

grep _spoor_runtime_LogFunctionEntry "$OUTPUT_IR_FILE" > /dev/null
grep _spoor_runtime_LogFunctionExit "$OUTPUT_IR_FILE" > /dev/null

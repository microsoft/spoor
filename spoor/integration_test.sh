#!/usr/bin/env bash
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set -e

BASE_PATH="spoor"
OUTPUT_EXECUTABLE_FILE="fib"
UNINSTRUMENTED_IR_FILE="fib.ll"
INSTRUMENTED_IR_FILE="fib_instrumented.ll"
OUTPUT_FUNCTION_MAP_FILE="fib.spoor_function_map"
OUTPUT_EXECUTABLE_FILE="fib"

if command -v clang++-12 &> /dev/null; then
  CLANGXX="clang++-12"
elif command -v clang++ &> /dev/null; then
  CLANGXX="clang++"
else
  echo "clang++ not found"
  exit 1
fi

"$CLANGXX" \
  "$BASE_PATH/test_data/fib.cc" \
  -std=c++11 \
  -g \
  -O0 \
  -emit-llvm \
  -S \
  -o "$UNINSTRUMENTED_IR_FILE"

"$BASE_PATH/instrumentation/spoor_opt" \
  "$UNINSTRUMENTED_IR_FILE" \
  --output_language=ir \
  --output_function_map_file="$OUTPUT_FUNCTION_MAP_FILE" \
  --output_file="$INSTRUMENTED_IR_FILE"

"$CLANGXX" \
  "$INSTRUMENTED_IR_FILE" \
  -L"$BASE_PATH/runtime" \
  -lspoor_runtime \
  -o "$OUTPUT_EXECUTABLE_FILE"

RESULT="$($OUTPUT_EXECUTABLE_FILE)"

EXPECTED_RESULT="13"
if [ "$RESULT" != "$EXPECTED_RESULT" ]; then
  echo "Unexpected result for Fibonacci(7)." \
    "Expected $EXPECTED_RESULT, got $RESULT."
  exit 1
fi

ls | grep "$OUTPUT_FUNCTION_MAP_FILE" > /dev/null
ls | grep ".spoor_trace" > /dev/null

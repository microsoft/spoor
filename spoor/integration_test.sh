#!/usr/bin/env bash
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set -e

BASE_PATH="spoor"
OUTPUT_EXECUTABLE_FILE="fib"
UNINSTRUMENTED_IR_FILE="fib.ll"
INSTRUMENTED_IR_FILE="fib_instrumented.ll"
OUTPUT_SYMBOLS_FILE="fib.spoor_symbols"
OUTPUT_EXECUTABLE_FILE="fib"
OUTPUT_PERFETTO_FILE="trace.perfetto"
TRACE_PROCESSOR_QUERY_FILE="query_result.txt"

if command -v clang++-11 &> /dev/null; then
  CLANGXX="clang++-11"
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
  --output_symbols_file="$OUTPUT_SYMBOLS_FILE" \
  --output_file="$INSTRUMENTED_IR_FILE"

"$CLANGXX" \
  "$INSTRUMENTED_IR_FILE" \
  -L"$BASE_PATH/runtime" \
  -lspoor_runtime \
  -o "$OUTPUT_EXECUTABLE_FILE"

FIB_RESULT="$($OUTPUT_EXECUTABLE_FILE)"

EXPECTED_FIB_RESULT="13"
if [ "$FIB_RESULT" != "$EXPECTED_FIB_RESULT" ]; then
  echo "Unexpected result for Fibonacci(7)." \
    "Expected $EXPECTED_FIB_RESULT, got $FIB_RESULT."
  exit 1
fi

"$BASE_PATH/tools/spoor" \
  *.spoor_trace \
  "$OUTPUT_SYMBOLS_FILE" \
  --output_file="$OUTPUT_PERFETTO_FILE"

echo "SELECT COUNT(id) FROM slice;" >> "$TRACE_PROCESSOR_QUERY_FILE"

TRACE_QUERY_RESULT="$(
  PERFETTO_TRACE_PROCESSOR_INSTALL_PATH="external/dev_perfetto_trace_processor" \
  "external/dev_perfetto_trace_processor/file/trace_processor" \
  "$OUTPUT_PERFETTO_FILE" \
  --query-file="$TRACE_PROCESSOR_QUERY_FILE" | tail -1)"

EXPECTED_TRACE_QUERY_RESULT="42"
if [ "$TRACE_QUERY_RESULT" != "$EXPECTED_TRACE_QUERY_RESULT" ]; then
  echo "Unexpected result for query '$(cat "$TRACE_PROCESSOR_QUERY_FILE")'." \
    "Expected '$EXPECTED_TRACE_QUERY_RESULT', got '$TRACE_QUERY_RESULT'."
  exit 1
fi

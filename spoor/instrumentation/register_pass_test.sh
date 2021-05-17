#!/usr/bin/env bash
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set -e

if command -v opt-12 &> /dev/null; then
  OPT="opt-12"
elif command -v opt &> /dev/null; then
  OPT="opt"
else
  echo "opt not found"
  exit 1
fi

BASE_PATH="spoor/instrumentation"

if [ "$(uname)" == "Darwin" ]; then
  PLUGIN="$BASE_PATH/libspoor_instrumentation.dylib"
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
  PLUGIN="$BASE_PATH/libspoor_instrumentation.so"
else
  echo "Unknown platform: $(uname)"
  exit 1
fi

OUTPUT_IR_FILE="fib_instrumented.ll"
export SPOOR_INSTRUMENTATION_OUTPUT_FUNCTION_MAP_FILE="fib.spoor_function_map"

# TODO(#131): Fix `opt` pass plugin support for IR instrumentation.
set +e
"$OPT" "$BASE_PATH/test_data/fib.ll" \
  -load-pass-plugin="$PLUGIN" \
  -passes="inject-spoor-instrumentation" \
  -S \
  -o "$OUTPUT_IR_FILE"
set -e

exit 0

if ! [[ -s "$SPOOR_INSTRUMENTATION_OUTPUT_FUNCTION_MAP_FILE" ]]; then
  echo "The function map file" \
      "'$SPOOR_INSTRUMENTATION_OUTPUT_FUNCTION_MAP_FILE' is empty or was not" \
      "created."
  exit 1
fi

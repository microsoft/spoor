#!/usr/bin/env bash

set -eu

BAZEL_ROOT="$(bazel info execution_root)"
WORKSPACE="$(bazel info workspace)"
COMPILE_COMMANDS="compile_commands.json"
COMPILE_COMMAND_GLOB="*.compile_command.pb"

if [ -d "$BAZEL_ROOT" ]; then 
  find "$BAZEL_ROOT" -type f -name "$COMPILE_COMMAND_GLOB" -delete
fi

bazel build \
  --experimental_action_listener=//toolchain/compilation_database:extract_compile_command_listener \
  --noshow_progress \
  --noshow_loading_progress \
  --output_groups=compilation_outputs \
  $(bazel query 'kind(cc_.*, //...)') # > /dev/null

find "$BAZEL_ROOT" -type f -name "$COMPILE_COMMAND_GLOB" |
  bazel run //toolchain/compilation_database:concatenate_compile_commands -- \
    --compile_command_directory="$BAZEL_ROOT" \
    --output_compilation_database="$WORKSPACE/$COMPILE_COMMANDS"

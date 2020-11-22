#!/usr/bin/env bash

set -e

WORKSPACE="$(bazel info workspace)"

if command -v clang-tidy-11 &> /dev/null; then
  CLANG_TIDY="clang-tidy-11"
elif command -v clang-tidy &> /dev/null; then
  CLANG_TIDY="clang-tidy"
else
  echo "clang-tidy not found"
  exit 1
fi

function run_clang_tidy {
  echo "Tidying $1"
  "$CLANG_TIDY" \
    -p="$WORKSPACE" \
    --checks='' \
    --config="$(cat "$WORKSPACE"/.clang-tidy)" \
    "$1"
}

echo "Generating compilation database"
./toolchain/compilation_database/generate_compilation_database.sh

echo "Building C++ targets"
# Building all C++ targets before running Clang Tidy ensures that
# `$(bazel info execution_root)` contains the necessary dependencies.
bazel build $(bazel query 'kind(cc_.*, //...)')

if [ $# -eq 0 ]; then
  # TODO(#42): Do not exclude `event_logger_notifier_mock.h`.
  # TODO(#37): Do not exclude `flush_queue_mock.h`.
  # TODO(#49): Do not exclude `trace_reader_mock.h`.
  # TODO(#34): Do not exclude `trace_writer_mock.h`.
  find "$WORKSPACE" \
    -type f \
    \( -iname "*.h" -o -iname "*.cc" \) \
    ! -name "event_logger_notifier_mock.h" \
    ! -name "flush_queue_mock.h" \
    ! -name "trace_reader_mock.h" \
    ! -name "trace_writer_mock.h" \
    -print0 |
      while read -d $'\0' file_name; do
        run_clang_tidy "$file_name"
      done
else
  for file_name in "$@"; do
    run_clang_tidy "$file_name"
  done
fi

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

if [[ "$AZURE_PIPELINES_OS" == "Linux" ]]; then
  echo "Detected Linux CI environment"
  # TODO(#17): Remove CI-specific warnings as errors.
  # Clang Tidy is mysteriously emitting the following violation in CI for most
  # source files the following error:
  # `error: invalid case style for template parameter 'expr-type'`.
  # The does not make sense because 'expr-type' is invalid template parameter
  # syntax and does not reproduce locally (on macOS or Ubuntu). The check will
  # not be treated as an error for not to unblock development.
  WARNING_AS_ERRORS="-readability-identifier-naming"
else
  WARNING_AS_ERRORS=""
fi

function run_clang_tidy {
  echo "Tidying $1"
  "$CLANG_TIDY" \
    -p="$WORKSPACE" \
    --checks='' \
    --warnings-as-errors="$WARNING_AS_ERRORS" \
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
  find "$WORKSPACE" \
    -type f \( -iname "*.h" -o -iname "*.cc" \) \
    -print0 |
      while read -d $'\0' file_name; do
        run_clang_tidy "$file_name"
      done
else
  for file_name in "$@"; do
    run_clang_tidy "$file_name"
  done
fi

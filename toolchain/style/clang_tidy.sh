#!/usr/bin/env bash

set -u

WORKSPACE="$(bazel info workspace)"

if command -v clang-tidy &> /dev/null; then
  CLANG_TIDY="clang-tidy"
elif command -v clang-tidy-10 &> /dev/null; then
  CLANG_TIDY="clang-tidy-10"
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
# Building all C++ targets before running clang-tidy ensures that
# $(bazel info execution_root) contains the necessary dependencies.
bazel build $(bazel query 'kind(cc_.*, //...)')

if [ $# -eq 0 ]; then
  find "$WORKSPACE" \
    -type f \
    \( -iname '*.h' -o -iname '*.cc' ! -iname 'bazel-' \) \
    -print0 |
      while read -d $'\0' file_name; do
        run_clang_tidy "$file_name"
      done
else
  for file_name in "$@"; do
    run_clang_tidy "$file_name"
  done
fi

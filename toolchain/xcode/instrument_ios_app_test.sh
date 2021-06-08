#!/usr/bin/env bash
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set -eu

export HOME="$(pwd)"
APP_PATH="$(pwd)/$APP_RELATIVE_PATH"
DESTINATION="platform=iOS Simulator,name=iPhone 12"
CONFIGURATION="Debug"
XCODE_TOOLCHAINS_PATH="$HOME/Library/Developer/Toolchains"
SPOOR_TOOLCHAIN_PATH="$(pwd)/toolchain/xcode/$TOOLCHAIN_NAME"
DERIVED_DATA_PATH="$(pwd)/DerivedData"
OBJECT_FILE_PATH="$DERIVED_DATA_PATH/Build/Intermediates.noindex/$APP_NAME.build/$CONFIGURATION-iphonesimulator/$APP_NAME.build/Objects-normal"
BINARY_FILE_PATH="$DERIVED_DATA_PATH/Build/Products/$CONFIGURATION-iphonesimulator/$APP_NAME.app/$APP_NAME"

function clean_up {
  rm -rf "$DERIVED_DATA_PATH"
  rm -rf "$XCODE_TOOLCHAINS_PATH"
}

# Manually clean up artifacts because the this test cannot be run in a sandbox.
trap clean_up SIGINT
trap clean_up EXIT
clean_up

# Hack (continued): Revert files back to their original name by undoing the
# `http_archive` patch.
find "$APP_PATH" -name '*__SPACE__*' -print0 |
  sort -rz |
    while read -d $'\0' f; do
      mv "$f" "$(dirname "$f")/$(basename "${f//__SPACE__/ }")"
    done

mkdir -p "$DERIVED_DATA_PATH"
mkdir -p "$XCODE_TOOLCHAINS_PATH"
ln -s "$SPOOR_TOOLCHAIN_PATH" "$XCODE_TOOLCHAINS_PATH/$TOOLCHAIN_NAME"

xcodebuild \
  clean build \
  -quiet \
  -configuration "$CONFIGURATION" \
  -destination "$DESTINATION" \
  -derivedDataPath "$DERIVED_DATA_PATH" \
  -clonedSourcePackagesDirPath "$APP_PATH" \
  -project "$APP_PATH/$APP_NAME.xcodeproj" \
  -scheme "$APP_NAME" \
  -toolchain Spoor

if ! find "$DERIVED_DATA_PATH" -name "*.spoor_symbols" | grep -q "."; then
  echo "No function maps were created."
  exit 1
fi

if ! nm -g "$BINARY_FILE_PATH" | grep -q "__spoor_runtime_"; then
  echo "The instrumented binary does not contain Spoor runtime symbols."
  exit 1
fi

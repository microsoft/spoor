#!/usr/bin/env bash
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set -eu

WORKSPACE="$(bazel info workspace)"
bazel run //toolchain/copyright_header:add_copyright_header -- "$WORKSPACE"

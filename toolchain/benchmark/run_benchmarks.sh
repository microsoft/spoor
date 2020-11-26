#!/usr/bin/env bash
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set -eu

bazel query 'kind(cc_binary, //...) intersect attr(name, ".*_benchmark", //...)' |
  xargs bazel run --config=benchmark

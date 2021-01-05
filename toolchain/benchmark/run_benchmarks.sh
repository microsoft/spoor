#!/usr/bin/env bash
# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set -eu

bazel query 'kind(cc_binary, //...) intersect attr(name, ".*_benchmark", //...)' |
  xargs -L 1 bazel run --config=benchmark

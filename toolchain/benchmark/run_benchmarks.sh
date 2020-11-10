#!/usr/bin/env bash

set -eu

bazel query 'kind(cc_binary, //...) intersect attr(name, ".*_benchmark", //...)' |
  xargs bazel run --config=benchmark

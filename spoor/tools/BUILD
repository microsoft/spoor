# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_python//python:defs.bzl", "py_binary")
load("@py_deps//:requirements.bzl", "requirement")

py_binary(
    name = "main",
    srcs = ["main.py"],
    deps = [
      "//spoor/proto:spoor_py_proto",
      # requirement("lz4"),
      requirement("numpy"),
    ],
)

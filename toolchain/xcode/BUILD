# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_python//python:defs.bzl", "py_binary", "py_library", "py_test")
load("@pip_deps_dev//:requirements.bzl", "requirement")

py_library(
    name = "hello_lib",
    srcs = ["hello_lib.py"],
)

py_binary(
    name = "hello",
    srcs = ["hello.py"],
    deps = [":hello_lib"],
)

py_test(
    name = "hello_test",
    srcs = ["hello_test.py"],
    deps = [
        ":hello_lib",
        requirement("pytest"),
    ],
)

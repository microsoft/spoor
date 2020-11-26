# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "numeric",
    hdrs = ["numeric.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "result",
    hdrs = ["result.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "result_test",
    size = "small",
    srcs = ["result_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        ":numeric",
        ":result",
        "@com_google_googletest//:gtest_main",
    ],
)

# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "config",
    srcs = ["config.cc"],
    hdrs = ["config.h"],
    copts = ["-Werror"],
    visibility = ["//spoor/runtime:__subpackages__"],
    deps = [
        "//spoor/runtime/buffer",
        "//spoor/runtime/trace",
        "//util:numeric",
        "//util/compression",
        "//util/env",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "config_test",
    size = "small",
    srcs = ["config_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":config",
        "@com_google_googletest//:gtest_main",
    ],
)

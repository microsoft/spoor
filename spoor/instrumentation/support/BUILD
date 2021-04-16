# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "support",
    srcs = ["support.cc"],
    hdrs = ["support.h"],
    visibility = ["//spoor/instrumentation:__pkg__"],
    deps = [
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "support_test",
    size = "small",
    srcs = ["support_test.cc"],
    visibility = ["//visibility:private"],
    deps = [
        ":support",
        "@com_google_googletest//:gtest_main",
    ],
)

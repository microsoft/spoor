# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "optional",
    srcs = ["optional.cc"],
    hdrs = ["optional.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "optional_test",
    size = "small",
    srcs = ["optional_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":optional",
        "//util:numeric",
        "@com_google_googletest//:gtest_main",
    ],
)

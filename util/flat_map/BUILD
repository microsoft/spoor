# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "flat_map",
    hdrs = ["flat_map.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = ["//util:result"],
)

cc_test(
    name = "flat_map_test",
    size = "small",
    srcs = ["flat_map_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        ":flat_map",
        "//util:numeric",
        "@com_google_googletest//:gtest_main",
    ],
)

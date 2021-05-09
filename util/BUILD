# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load(
    "//toolchain:config.bzl",
    "SPOOR_DEFAULT_COPTS",
    "SPOOR_DEFAULT_LINKOPTS",
    "SPOOR_DEFAULT_TEST_COPTS",
    "SPOOR_DEFAULT_TEST_LINKOPTS",
)

cc_library(
    name = "numeric",
    hdrs = ["numeric.h"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "result",
    hdrs = ["result.h"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:public"],
)

cc_test(
    name = "result_test",
    size = "small",
    srcs = ["result_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":numeric",
        ":result",
        "@com_google_googletest//:gtest_main",
    ],
)

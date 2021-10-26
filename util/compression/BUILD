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
    name = "compression",
    srcs = [
        "compressor_factory.cc",
        "none_compressor.cc",
        "snappy_compressor.cc",
    ],
    hdrs = [
        "compressor.h",
        "compressor_factory.h",
        "none_compressor.h",
        "snappy_compressor.h",
    ],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//util:numeric",
        "//util:result",
        "//util/flat_map",
        "@com_google_snappy//:snappy",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "compressor_test",
    size = "small",
    srcs = ["compressor_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":compression",
        "@com_google_googletest//:gtest_main",
        "@com_microsoft_gsl//:gsl",
    ],
)

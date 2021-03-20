# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

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
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "//util:numeric",
        "//util:result",
        "@com_google_snappy//:snappy",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "compressor_test",
    size = "small",
    srcs = ["compressor_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":compression",
        "@com_google_googletest//:gtest_main",
        "@com_microsoft_gsl//:gsl",
    ],
)

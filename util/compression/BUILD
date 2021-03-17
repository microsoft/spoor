# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

cmake(
    name = "snappy",
    lib_source = "@com_google_snappy//:all_srcs",
    out_static_libs = ["libsnappy.a"],
    visibility = ["//visibility:private"],
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
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        ":snappy",
        "//util:numeric",
        "//util:result",
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

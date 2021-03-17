# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "serialization",
    hdrs = ["serializer.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "//util:result",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "serialization_test",
    size = "small",
    srcs = ["serialization_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":serialization",
        "//util:numeric",
        "@com_google_googletest//:gtest_main",
        "@com_microsoft_gsl//:gsl",
    ],
)

# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load("//toolchain:cc_static_library.bzl", "cc_static_library")

cc_library(
    name = "runtime",
    srcs = ["runtime.cc"],
    hdrs = ["runtime.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/runtime/config",
        "//spoor/runtime/event_logger",
        "//spoor/runtime/runtime_manager",
        "//spoor/runtime/trace",
        "//util:numeric",
        "@com_google_absl//absl/strings",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_static_library(
    name = "spoor_runtime",
    deps = [":runtime"],
)

cc_test(
    name = "runtime_test",
    size = "small",
    srcs = ["runtime_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":runtime",
        "@com_google_googletest//:gtest_main",
        "@com_microsoft_gsl//:gsl",
    ],
)

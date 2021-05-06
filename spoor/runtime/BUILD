# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load("//toolchain:cc_static_library.bzl", "cc_static_library")

cc_library(
    name = "runtime",
    srcs = [
        "runtime.cc",
        "runtime_common.cc",
    ],
    hdrs = ["runtime.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/runtime/config",
        "//spoor/runtime/event_logger",
        "//spoor/runtime/runtime_manager",
        "//spoor/runtime/trace",
        "//util:numeric",
        "//util/compression",
        "@com_google_absl//absl/strings",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_static_library(
    name = "spoor_runtime",
    visibility = ["//visibility:public"],
    deps = [":runtime"],
)

cc_library(
    name = "runtime_stub",
    srcs = [
        "runtime_common.cc",
        "runtime_stub.cc",
    ],
    hdrs = ["runtime.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = ["@com_microsoft_gsl//:gsl"],
)

cc_static_library(
    name = "spoor_runtime_stub",
    visibility = ["//visibility:public"],
    deps = [":runtime_stub"],
)

cc_test(
    name = "runtime_test",
    size = "small",
    srcs = [
        "runtime_common_test.cc",
        "runtime_test.cc",
    ],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":runtime",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "runtime_stub_test",
    size = "small",
    srcs = ["runtime_stub_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_stub",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
        "@com_microsoft_gsl//:gsl",
    ],
)

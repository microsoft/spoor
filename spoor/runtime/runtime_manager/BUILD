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
    name = "runtime_manager",
    srcs = ["runtime_manager.cc"],
    hdrs = ["runtime_manager.h"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//spoor/runtime:__pkg__"],
    deps = [
        "//spoor/runtime/buffer",
        "//spoor/runtime/event_logger",
        "//spoor/runtime/flush_queue",
        "//spoor/runtime/trace",
        "//util:numeric",
        "//util/file_system",
        "//util/time:clock",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "runtime_manager_test",
    size = "small",
    srcs = ["runtime_manager_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_manager",
        "//spoor/runtime/buffer",
        "//spoor/runtime/event_logger",
        "//spoor/runtime/flush_queue:flush_queue_mock",
        "//spoor/runtime/trace",
        "//spoor/runtime/trace:trace_mock",
        "//util/file_system:file_system_mock",
        "//util/time:clock_mock",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_binary(
    name = "runtime_manager_benchmark",
    srcs = ["runtime_manager_benchmark.cc"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_manager",
        "//spoor/runtime/flush_queue:black_hole_flush_queue",
        "//util:numeric",
        "//util/time:clock",
        "@com_google_benchmark//:benchmark",
        "@com_microsoft_gsl//:gsl",
    ],
)

# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "event_logger",
    srcs = ["event_logger.cc"],
    hdrs = [
        "event_logger.h",
        "event_logger_notifier.h",
    ],
    copts = ["-Werror"],
    visibility = ["//spoor/runtime:__subpackages__"],
    deps = [
        "//spoor/runtime/buffer",
        "//spoor/runtime/flush_queue",
        "//spoor/runtime/trace",
        "//util/time:clock",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_library(
    name = "event_logger_notifier_mock",
    testonly = True,
    hdrs = ["event_logger_notifier_mock.h"],
    copts = ["-Werror"],
    visibility = ["//spoor/runtime:__subpackages__"],
    deps = [
        ":event_logger",
        "@com_google_googletest//:gtest",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "event_logger_test",
    size = "small",
    srcs = ["event_logger_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":event_logger",
        ":event_logger_notifier_mock",
        "//spoor/runtime/buffer",
        "//spoor/runtime/flush_queue:flush_queue_mock",
        "//spoor/runtime/trace",
        "//util/time:clock_mock",
        "@com_google_googletest//:gtest_main",
        "@com_microsoft_gsl//:gsl",
    ],
)

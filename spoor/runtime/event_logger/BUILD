load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "event_logger",
    hdrs = [
        "event_logger.h",
        "event_logger_notifier.h",
    ],
    srcs = ["event_logger.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/runtime/buffer:buffer",
        "//spoor/runtime/flush_queue:flush_queue",
        "//spoor/runtime/trace:trace",
        "//util/time:clock",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_library(
    name = "event_logger_notifier_mock",
    hdrs = ["event_logger_notifier_mock.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        ":event_logger",
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
        "//spoor/runtime/buffer:buffer",
        "//spoor/runtime/flush_queue:flush_queue_mock",
        "//spoor/runtime/trace:trace",
        "//util/time:clock_mock",
        "@com_google_googletest//:gtest_main",
        "@com_microsoft_gsl//:gsl",
    ],
)

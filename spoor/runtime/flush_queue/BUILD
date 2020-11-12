load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "flush_queue",
    srcs = ["disk_flush_queue.cc"],
    hdrs = [
        "disk_flush_queue.h",
        "flush_queue.h",
    ],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/runtime/buffer",
        "//spoor/runtime/trace",
        "//util:numeric",
        "//util/time:clock",
        "@com_google_absl//absl/strings:str_format",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "disk_flush_queue_test",
    size = "small",
    srcs = ["disk_flush_queue_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":flush_queue",
        "//spoor/runtime/buffer",
        "//spoor/runtime/trace",
        "//spoor/runtime/trace:trace_writer_mock",
        "//util:numeric",
        "//util/time:clock_mock",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_googletest//:gtest_main",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_library(
    name = "flush_queue_mock",
    hdrs = ["flush_queue_mock.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        ":flush_queue",
        "@com_google_googletest//:gtest",
    ],
)

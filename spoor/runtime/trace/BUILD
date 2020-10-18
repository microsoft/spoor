load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "trace",
    srcs = [
        "trace_writer.cc",
    ],
    hdrs = [
        "trace.h",
        "trace_writer.h",
    ],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/runtime/buffer",
        "//util:numeric",
        "@com_google_absl//absl/base:endian",
        "@com_google_absl//absl/strings",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "trace_test",
    size = "small",
    srcs = ["trace_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":trace",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "trace_writer_mock",
    hdrs = [
        "trace_writer_mock.h",
    ],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        ":trace",
        "@com_google_googletest//:gtest_main",
    ],
)

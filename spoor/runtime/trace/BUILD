load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "trace",
    srcs = ["trace.cc"],
    hdrs = ["trace.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = ["//util:numeric"],
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

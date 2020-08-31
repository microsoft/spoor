load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

cc_library(
    name = "lib",
    srcs = ["lib.cc"],
    hdrs = ["lib.h"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "lib_test",
    size = "small",
    srcs = ["lib_test.cc"],
    visibility = ["//visibility:private"],
    deps = [
        ":lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_binary(
    name = "bin",
    srcs = ["main.cc"],
    visibility = ["//visibility:public"],
    deps = [":lib"],
)

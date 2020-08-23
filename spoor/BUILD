load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

cc_library(
    name = "lib",
    srcs = ["lib.cc"],
    hdrs = ["lib.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = ["//util:numeric"],
)

cc_test(
    name = "lib_test",
    size = "small",
    srcs = ["lib_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_binary(
    name = "bin",
    srcs = ["main.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [":lib"],
)

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "ifstream",
    hdrs = ["ifstream.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "ifstream_test",
    size = "small",
    srcs = ["ifstream_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":ifstream",
        "@com_google_googletest//:gtest_main",
    ],
)

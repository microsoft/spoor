load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "env",
    hdrs = ["env.h"],
    srcs = ["env.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "numeric",
    hdrs = ["numeric.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "result",
    hdrs = ["result.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "env_test",
    size = "small",
    srcs = ["env_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":env",
        ":numeric",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "result_test",
    size = "small",
    srcs = ["result_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":numeric",
        ":result",
        "@com_google_googletest//:gtest_main",
    ],
)

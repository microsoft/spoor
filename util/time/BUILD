# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cc_library(
    name = "clock",
    hdrs = ["clock.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "clock_test",
    size = "small",
    srcs = ["clock_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":clock",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "clock_mock",
    hdrs = ["clock_mock.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        ":clock",
        "//util:numeric",
        "@com_google_googletest//:gtest",
    ],
)

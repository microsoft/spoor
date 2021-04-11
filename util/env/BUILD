# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cc_library(
    name = "env",
    srcs = ["env.cc"],
    hdrs = ["env.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "//util/flat_map",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "env_test",
    size = "small",
    srcs = ["env_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":env",
        "//util:numeric",
        "//util/flat_map",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

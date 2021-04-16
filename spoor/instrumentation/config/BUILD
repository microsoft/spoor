# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "config",
    srcs = ["config.cc"],
    hdrs = ["config.h"],
    copts = ["-Werror"],
    visibility = ["//spoor/instrumentation:__pkg__"],
    deps = [
        "//util:numeric",
        "//util/flags:optional",
        "//util/flat_map",
    ],
)

cc_library(
    name = "env_config",
    srcs = ["env_config.cc"],
    hdrs = ["env_config.h"],
    copts = ["-Werror"],
    visibility = ["//spoor/instrumentation:__pkg__"],
    deps = [
        ":config",
        "//util:numeric",
        "//util/env",
    ],
)

cc_library(
    name = "command_line_config",
    srcs = ["command_line_config.cc"],
    hdrs = ["command_line_config.h"],
    copts = ["-Werror"],
    visibility = ["//spoor/instrumentation:__pkg__"],
    deps = [
        ":config",
        ":env_config",
        "//util:numeric",
        "//util/env",
        "//util/flags:optional",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "config_test",
    size = "small",
    srcs = ["config_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":config",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "env_config_test",
    size = "small",
    srcs = ["env_config_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":config",
        ":env_config",
        "//util:numeric",
        "//util/flat_map",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "command_line_config_test",
    size = "small",
    srcs = ["command_line_config_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":command_line_config",
        ":config",
        ":env_config",
        "//util:numeric",
        "@com_google_googletest//:gtest_main",
    ],
)

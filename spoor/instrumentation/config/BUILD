# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load(
    "//toolchain:config.bzl",
    "SPOOR_DEFAULT_COPTS",
    "SPOOR_DEFAULT_LINKOPTS",
    "SPOOR_DEFAULT_TEST_COPTS",
    "SPOOR_DEFAULT_TEST_LINKOPTS",
)

cc_library(
    name = "output_language",
    hdrs = ["output_language.h"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//spoor/instrumentation:__subpackages__"],
    deps = ["//util/flat_map"],
)

cc_test(
    name = "output_language_test",
    size = "small",
    srcs = ["output_language_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":output_language",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "source",
    srcs = [
        "env_source.cc",
        "file_source.cc",
    ],
    hdrs = [
        "env_source.h",
        "file_source.h",
        "source.h",
    ],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//spoor/instrumentation:__pkg__"],
    deps = [
        ":output_language",
        "//util:result",
        "//util/env",
        "//util/file_system",
        "//util/flags:optional",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_marzer_tomlplusplus//:tomlplusplus",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_library(
    name = "source_mock",
    testonly = True,
    hdrs = ["source_mock.h"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":source",
        "@com_google_googletest//:gtest",
    ],
)

cc_test(
    name = "source_test",
    size = "small",
    srcs = [
        "env_source_test.cc",
        "file_source_test.cc",
    ],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":output_language",
        ":source",
        "//util/env",
        "//util/file_system",
        "//util/file_system:file_system_mock",
        "//util/flat_map",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "config_private",
    hdrs = ["config_private.h"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [":output_language"],
)

cc_library(
    name = "config",
    srcs = [
        "command_line.cc",
        "config.cc",
    ],
    hdrs = [
        "command_line.h",
        "config.h",
    ],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//spoor/instrumentation:__pkg__"],
    deps = [
        ":config_private",
        ":output_language",
        ":source",
        "//spoor/common/config:util",
        "//util/file_system",
    ],
)

cc_test(
    name = "config_test",
    size = "small",
    srcs = [
        "command_line_test.cc",
        "config_test.cc",
    ],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":config",
        ":output_language",
        ":source",
        ":source_mock",
        "//util/env",
        "//util/file_system",
        "//util/flat_map",
        "@com_google_googletest//:gtest_main",
    ],
)

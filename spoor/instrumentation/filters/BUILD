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
    name = "filters",
    srcs = ["filters.cc"],
    hdrs = ["filters.h"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//spoor/instrumentation:__subpackages__"],
    deps = [
        "//util:numeric",
    ],
)

cc_test(
    name = "filters_test",
    size = "small",
    srcs = ["filters_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":filters",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "filters_reader",
    srcs = ["filters_file_reader.cc"],
    hdrs = [
        "filters_file_reader.h",
        "filters_reader.h",
    ],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//spoor/instrumentation:__subpackages__"],
    deps = [
        ":filters",
        "//util:numeric",
        "//util/file_system",
        "//util/flat_map",
        "@com_google_absl//absl/strings:str_format",
        "@com_marzer_tomlplusplus//:tomlplusplus",
    ],
)

cc_test(
    name = "filters_file_reader_test",
    size = "small",
    srcs = ["filters_file_reader_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":filters",
        ":filters_reader",
        "//util/file_system:file_system_mock",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "filters_mock",
    testonly = True,
    hdrs = ["filters_reader_mock.h"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//spoor/instrumentation:__subpackages__"],
    deps = [
        ":filters",
        "//util:numeric",
        "@com_google_googletest//:gtest",
    ],
)

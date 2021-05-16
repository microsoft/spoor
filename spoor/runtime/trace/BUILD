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
    name = "trace",
    srcs = [
        "trace_file_reader.cc",
        "trace_file_writer.cc",
    ],
    hdrs = [
        "trace.h",
        "trace_file_reader.h",
        "trace_file_writer.h",
        "trace_reader.h",
        "trace_writer.h",
    ],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/runtime/buffer",
        "//util:numeric",
        "//util/compression",
        "//util/file_system",
        "@com_google_absl//absl/base:endian",
        "@com_google_absl//absl/strings",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "trace_test",
    size = "small",
    srcs = ["trace_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":trace",
        "//spoor/runtime/buffer",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "trace_reader_test",
    size = "small",
    srcs = ["trace_file_reader_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":trace",
        "//util:result",
        "//util/compression",
        "//util/file_system:file_system_mock",
        "//util/flat_map",
        "@com_google_absl//absl/base:endian",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "trace_writer_test",
    size = "small",
    srcs = ["trace_file_writer_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":trace",
        "//util/compression",
        "//util/file_system:file_system_mock",
        "@com_google_googletest//:gtest_main",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_library(
    name = "trace_mock",
    testonly = True,
    hdrs = [
        "trace_reader_mock.h",
        "trace_writer_mock.h",
    ],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//spoor/runtime:__subpackages__"],
    deps = [
        ":trace",
        "@com_google_googletest//:gtest",
    ],
)

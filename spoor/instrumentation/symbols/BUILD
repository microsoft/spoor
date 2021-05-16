# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_proto_library", "cc_test")
load("@rules_proto//proto:defs.bzl", "proto_library")
load(
    "//toolchain:config.bzl",
    "SPOOR_DEFAULT_COPTS",
    "SPOOR_DEFAULT_LINKOPTS",
    "SPOOR_DEFAULT_TEST_COPTS",
    "SPOOR_DEFAULT_TEST_LINKOPTS",
)

proto_library(
    name = "symbols_proto",
    srcs = ["symbols.proto"],
    visibility = ["//visibility:public"],
    deps = ["@com_google_protobuf//:timestamp_proto"],
)

cc_proto_library(
    name = "symbols_cc_proto",
    visibility = ["//visibility:public"],
    deps = [":symbols_proto"],
)

cc_library(
    name = "symbols_reader",
    srcs = ["symbols_file_reader.cc"],
    hdrs = [
        "symbols_file_reader.h",
        "symbols_reader.h",
    ],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":symbols_cc_proto",
        "//util:result",
        "//util/file_system",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "symbols_reader_test",
    size = "small",
    srcs = ["symbols_file_reader_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":symbols_cc_proto",
        ":symbols_reader",
        "//util/file_system:file_system_mock",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "symbols_writer",
    srcs = ["symbols_file_writer.cc"],
    hdrs = [
        "symbols_file_writer.h",
        "symbols_writer.h",
    ],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":symbols_cc_proto",
        "//util:result",
        "//util/file_system",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "symbols_writer_test",
    size = "small",
    srcs = ["symbols_file_writer_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":symbols_cc_proto",
        ":symbols_writer",
        "//util/file_system:file_system_mock",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "symbols_mock",
    testonly = True,
    hdrs = [
        "symbols_reader_mock.h",
        "symbols_writer_mock.h",
    ],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//spoor/instrumentation:__subpackages__"],
    deps = [
        ":symbols_cc_proto",
        ":symbols_reader",
        ":symbols_writer",
        "@com_google_googletest//:gtest",
    ],
)

cc_library(
    name = "symbols_utility",
    srcs = ["symbols_utility.cc"],
    hdrs = ["symbols_utility.h"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":symbols_cc_proto",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "symbols_utility_test",
    size = "small",
    srcs = ["symbols_utility_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":symbols_cc_proto",
        ":symbols_utility",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
)

# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library")
load(
    "//toolchain:config.bzl",
    "SPOOR_DEFAULT_COPTS",
    "SPOOR_DEFAULT_LINKOPTS",
    "SPOOR_DEFAULT_TEST_COPTS",
    "SPOOR_DEFAULT_TEST_LINKOPTS",
)

cc_library(
    name = "file_system",
    srcs = [
        "local_file_reader.cc",
        "local_file_system.cc",
        "local_file_writer.cc",
    ],
    hdrs = [
        "file_reader.h",
        "file_system.h",
        "file_writer.h",
        "local_file_reader.h",
        "local_file_system.h",
        "local_file_writer.h",
    ],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//util:result",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_library(
    name = "file_system_mock",
    srcs = ["directory_entry_mock.cc"],
    hdrs = [
        "directory_entry_mock.h",
        "file_reader_mock.h",
        "file_system_mock.h",
        "file_writer_mock.h",
    ],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":file_system",
        "//util:result",
        "@com_google_googletest//:gtest",
        "@com_microsoft_gsl//:gsl",
    ],
)

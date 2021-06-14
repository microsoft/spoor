# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")
load(
    "//toolchain:config.bzl",
    "SPOOR_DEFAULT_COPTS",
    "SPOOR_DEFAULT_LINKOPTS",
    "SPOOR_DEFAULT_TEST_COPTS",
    "SPOOR_DEFAULT_TEST_LINKOPTS",
)

cc_library(
    name = "tools_info",
    hdrs = ["tools.h"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:private"],
)

cc_binary(
    name = "spoor",
    srcs = ["spoor.cc"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":tools_info",
        "//spoor/instrumentation:instrumentation_info",
        "//spoor/instrumentation/symbols:symbols_cc_proto",
        "//spoor/instrumentation/symbols:symbols_reader",
        "//spoor/instrumentation/symbols:symbols_utility",
        "//spoor/instrumentation/symbols:symbols_writer",
        "//spoor/runtime/trace",
        "//spoor/tools/config",
        "//spoor/tools/config:command_line_config",
        "//spoor/tools/serialization:serialize",
        "//util:numeric",
        "//util/compression",
        "//util/env",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_microsoft_gsl//:gsl",
    ],
)

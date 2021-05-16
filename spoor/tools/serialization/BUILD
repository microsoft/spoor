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
    name = "serialize",
    srcs = ["serialize.cc"],
    hdrs = ["serialize.h"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/instrumentation:instrumentation_info",
        "//spoor/instrumentation/symbols:symbols_cc_proto",
        "//spoor/runtime/trace",
        "//spoor/tools/adapters/perfetto:perfetto_adapter",
        "//spoor/tools/config",
        "//util:result",
        "@com_google_absl//absl/strings",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "serialize_test",
    size = "small",
    srcs = ["serialize_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":serialize",
        "//spoor/instrumentation/symbols:symbols_cc_proto",
        "//spoor/runtime/trace",
        "//spoor/tools/adapters/perfetto:perfetto_trace_cc_proto",
        "//spoor/tools/config",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
)

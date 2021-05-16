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

cc_proto_library(
    name = "perfetto_trace_cc_proto",
    visibility = ["//visibility:public"],
    deps = ["@perfetto//:protos_perfetto_trace_non_minimal_protos"],
)

cc_library(
    name = "perfetto_adapter_private",
    srcs = ["perfetto_adapter_private.cc"],
    hdrs = ["perfetto_adapter_private.h"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        "//spoor/runtime/trace",
        "//util:numeric",
        "@com_google_cityhash//:city_hash",
    ],
)

cc_library(
    name = "perfetto_adapter",
    srcs = ["perfetto_adapter.cc"],
    hdrs = ["perfetto_adapter.h"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":perfetto_adapter_private",
        ":perfetto_trace_cc_proto",
        "//spoor/instrumentation/symbols:symbols_cc_proto",
        "//spoor/runtime/trace",
        "//util:numeric",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "perfetto_adapter_test",
    size = "small",
    srcs = ["perfetto_adapter_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":perfetto_adapter",
        ":perfetto_adapter_private",
        ":perfetto_trace_cc_proto",
        "//spoor/instrumentation/symbols:symbols_cc_proto",
        "//spoor/runtime/trace",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_cityhash//:city_hash",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
)

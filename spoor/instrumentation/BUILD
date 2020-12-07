# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_proto_library", "cc_test")
load("@rules_proto//proto:defs.bzl", "proto_library")

proto_library(
    name = "instrumentation_map_proto",
    srcs = ["instrumentation_map.proto"],
    visibility = ["//visibility:public"],
)

cc_proto_library(
    name = "instrumentation_map_cc_proto",
    visibility = ["//visibility:public"],
    deps = [":instrumentation_map_proto"],
)

cc_library(
    name = "instrumentation",
    srcs = [
        "instrumentation.h",
        "register_pass.cc",
    ],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        "//spoor/instrumentation/inject_runtime",
        "@llvm//11.0.0",
    ],
)

cc_test(
    name = "register_pass_test",
    size = "small",
    srcs = ["register_pass_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":instrumentation",
        "@com_google_googletest//:gtest_main",
        "@llvm//11.0.0",
    ],
)

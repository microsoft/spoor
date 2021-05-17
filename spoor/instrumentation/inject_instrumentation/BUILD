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
    name = "inject_instrumentation",
    srcs = ["inject_instrumentation.cc"],
    hdrs = ["inject_instrumentation.h"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//spoor/instrumentation:__pkg__"],
    deps = [
        "//spoor/proto:spoor_cc_proto",
        "//util:numeric",
        "//util/time:clock",
        "@com_apple_swift//:demangle",
        "@com_google_cityhash//:city_hash",
        "@com_microsoft_gsl//:gsl",
        "@llvm-project//llvm:Core",
        "@llvm-project//llvm:Demangle",
        "@llvm-project//llvm:Passes",
        "@llvm-project//llvm:Support",
    ],
)

cc_test(
    name = "inject_instrumentation_test",
    size = "small",
    srcs = ["inject_instrumentation_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    data = ["//spoor/instrumentation/test_data"],
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":inject_instrumentation",
        "//spoor/proto:spoor_cc_proto",
        "//util:numeric",
        "//util/time:clock",
        "//util/time:clock_mock",
        "@com_google_cityhash//:city_hash",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
        "@com_microsoft_gsl//:gsl",
        "@llvm-project//llvm:Core",
        "@llvm-project//llvm:IRReader",
        "@llvm-project//llvm:Support",
    ],
)

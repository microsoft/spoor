# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "inject_runtime",
    srcs = ["inject_runtime.cc"],
    hdrs = ["inject_runtime.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/instrumentation:instrumentation_map_cc_proto",
        "//util:numeric",
        "@com_apple_swift//:demangle",
        "@com_google_absl//absl/strings:str_format",
        "@com_microsoft_gsl//:gsl",
        "@llvm//11.0.0",
    ],
)

cc_test(
    name = "inject_runtime_test",
    size = "small",
    srcs = ["inject_runtime_test.cc"],
    copts = ["-Werror"],
    data = ["//spoor/instrumentation/inject_runtime/test_data"],
    visibility = ["//visibility:private"],
    deps = [
        ":inject_runtime",
        "//spoor/instrumentation:instrumentation_map_cc_proto",
        "//util:numeric",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
        "@com_microsoft_gsl//:gsl",
        "@llvm//11.0.0",
    ],
)

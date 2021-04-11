# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "inject_runtime",
    srcs = ["inject_runtime.cc"],
    hdrs = ["inject_runtime.h"],
    copts = ["-Werror"],
    visibility = ["//spoor/instrumentation:__pkg__"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_cityhash//:city_hash",
        "//spoor/proto:spoor_cc_proto",
        "//util:numeric",
        "//util/time:clock",
        "@com_apple_swift//:demangle",
        "@com_microsoft_gsl//:gsl",
        "@llvm//11.0.0",
    ],
)

cc_test(
    name = "inject_runtime_test",
    size = "small",
    srcs = ["inject_runtime_test.cc"],
    copts = ["-Werror"],
    data = ["//spoor/instrumentation/test_data"],
    visibility = ["//visibility:private"],
    deps = [
        ":inject_runtime",
        "//spoor/proto:spoor_cc_proto",
        "//util:numeric",
        "//util/time:clock",
        "//util/time:clock_mock",
        "@com_google_cityhash//:city_hash",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
        "@com_microsoft_gsl//:gsl",
        "@llvm//11.0.0",
    ],
)

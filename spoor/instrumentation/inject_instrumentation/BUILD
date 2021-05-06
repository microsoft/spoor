# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "inject_instrumentation",
    srcs = ["inject_instrumentation.cc"],
    hdrs = ["inject_instrumentation.h"],
    copts = ["-Werror"],
    visibility = ["//spoor/instrumentation:__pkg__"],
    deps = [
        "//spoor/proto:spoor_cc_proto",
        "//util:numeric",
        "//util/time:clock",
        "@com_apple_swift//:demangle",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_cityhash//:city_hash",
        "@com_microsoft_gsl//:gsl",
        "@llvm//12.0.0",
    ],
)

cc_test(
    name = "inject_instrumentation_test",
    size = "small",
    srcs = ["inject_instrumentation_test.cc"],
    copts = ["-Werror"],
    data = ["//spoor/instrumentation/test_data"],
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
        "@llvm//12.0.0",
    ],
)

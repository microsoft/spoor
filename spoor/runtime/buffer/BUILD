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
    name = "buffer",
    hdrs = [
        "buffer_slice_pool.h",
        "circular_buffer.h",
        "circular_slice_buffer.h",
        "combined_buffer_slice_pool.h",
        "dynamic_buffer_slice_pool.h",
        "owned_buffer_slice.h",
        "reserved_buffer_slice_pool.h",
        "unowned_buffer_slice.h",
    ],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//spoor/runtime:__subpackages__"],
    deps = [
        "//util:numeric",
        "//util:result",
        "//util/memory",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "circular_buffer_test",
    size = "small",
    srcs = ["circular_buffer_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
        "//util:numeric",
        "//util/memory",
        "@com_google_googletest//:gtest_main",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "buffer_slice_test",
    size = "small",
    srcs = ["buffer_slice_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
        "//util:numeric",
        "//util/memory",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "buffer_slice_pool_test",
    size = "small",
    srcs = ["buffer_slice_pool_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
        "//util:numeric",
        "//util/memory",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "dynamic_buffer_slice_pool_test",
    size = "small",
    srcs = ["dynamic_buffer_slice_pool_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
        "//util:numeric",
        "//util:result",
        "//util/memory",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "reserved_buffer_slice_pool_test",
    size = "small",
    srcs = ["reserved_buffer_slice_pool_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
        "//util:numeric",
        "//util/memory",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "combined_buffer_slice_pool_test",
    size = "small",
    srcs = ["combined_buffer_slice_pool_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
        "//util:numeric",
        "//util:result",
        "//util/memory",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "circular_slice_buffer_test",
    size = "small",
    srcs = ["circular_slice_buffer_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
        "//util:numeric",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_binary(
    name = "circular_slice_buffer_benchmark",
    srcs = ["circular_slice_buffer_benchmark.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
        "//util:numeric",
        "@com_google_benchmark//:benchmark",
        "@com_microsoft_gsl//:gsl",
    ],
)

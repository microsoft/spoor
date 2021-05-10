# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "objc_library")
load("@build_bazel_rules_apple//apple:ios.bzl", "ios_unit_test")
load("@build_bazel_rules_apple//apple:macos.bzl", "macos_unit_test")
load("//toolchain:config.bzl", "SPOOR_DEFAULT_COPTS", "SPOOR_DEFAULT_TEST_COPTS")

_MINIMUM_IOS_VERSION = "13.0"

_MINIMUM_MACOS_VERSION = "10.15"

_BUILD_COPTS = SPOOR_DEFAULT_COPTS + [
    "-std=c++2a",
]

_TEST_COPTS = SPOOR_DEFAULT_TEST_COPTS + [
     "-std=c++2a",
    "-Wno-gnu-statement-expression",
]


objc_library(
    name = "runtime_stub",
    srcs = [
        "SpoorConfig.mm",
        "SpoorConfig_private.h",
        "SpoorDeletedFilesInfo.mm",
        "SpoorDeletedFilesInfo_private.h",
        "SpoorRuntime.mm",
    ],
    hdrs = [
        "SpoorConfig.h",
        "SpoorDeletedFilesInfo.h",
        "SpoorRuntime.h",
        "SpoorTypes.h",
    ],
    module_name = "SpoorRuntime",
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/runtime:runtime_stub",
    ],
    copts = _BUILD_COPTS,
)

# Tests

ios_unit_test(
    name = "ios_runtime_wrapper_test",
    minimum_os_version = _MINIMUM_IOS_VERSION,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_wrapper_test_lib",
    ],
)

macos_unit_test(
    name = "macos_runtime_wrapper_test",
    minimum_os_version = _MINIMUM_MACOS_VERSION,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_wrapper_test_lib",
    ],
)

ios_unit_test(
    name = "ios_runtime_stub_test",
    minimum_os_version = _MINIMUM_IOS_VERSION,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_stub_test_lib",
    ],
)

macos_unit_test(
    name = "macos_runtime_stub_test",
    minimum_os_version = _MINIMUM_MACOS_VERSION,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_stub_test_lib",
    ],
)

objc_library(
    name = "runtime_wrapper_test_lib",
    testonly = True,
    srcs = [
        "SpoorConfigTests.mm",
        "SpoorDeletedFilesInfoTests.mm",
        "SpoorTypesTests.mm",
    ],
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_stub",
    ],
    copts = _TEST_COPTS,
)

objc_library(
    name = "runtime_stub_test_lib",
    testonly = True,
    srcs = [
        "SpoorRuntimeStubTests.mm",
    ],
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_stub",
    ],
    copts = _TEST_COPTS,
)

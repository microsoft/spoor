# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "objc_library")
load("@build_bazel_rules_apple//apple:ios.bzl", "ios_unit_test")
load("@build_bazel_rules_apple//apple:macos.bzl", "macos_unit_test")
load("//toolchain:config.bzl", "SPOOR_DEFAULT_COPTS", "SPOOR_DEFAULT_TEST_COPTS")

_MINIMUM_IOS_VERSION = "13.0"

_MINIMUM_MACOS_VERSION = "10.15"

_LIB_COPTS = SPOOR_DEFAULT_COPTS + [
    "-std=c++2a",
]

_TEST_COPTS = SPOOR_DEFAULT_TEST_COPTS + [
    "-std=c++2a",
    "-Wno-gnu-statement-expression",
]

_LIB_HDRS = [
    "SpoorConfig.h",
    "SpoorDeletedFilesInfo.h",
    "SpoorRuntime.h",
    "SpoorTypes.h",
]

_LIB_SRCS = [
    "SpoorConfig.mm",
    "SpoorConfig_private.h",
    "SpoorDeletedFilesInfo.mm",
    "SpoorDeletedFilesInfo_private.h",
    "SpoorRuntime.mm",
]

objc_library(
    name = "runtime",
    srcs = _LIB_SRCS,
    hdrs = _LIB_HDRS,
    copts = _LIB_COPTS,
    module_name = "SpoorRuntime",
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/runtime",
    ],
)

objc_library(
    name = "runtime_stub",
    srcs = _LIB_SRCS,
    hdrs = _LIB_HDRS,
    copts = _LIB_COPTS,
    module_name = "SpoorRuntimeStub",
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/runtime:runtime_stub",
    ],
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
    name = "ios_runtime_test",
    minimum_os_version = _MINIMUM_IOS_VERSION,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_test_lib",
    ],
)

macos_unit_test(
    name = "macos_runtime_test",
    minimum_os_version = _MINIMUM_MACOS_VERSION,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_test_lib",
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
    copts = _TEST_COPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_stub",
    ],
)

objc_library(
    name = "runtime_test_lib",
    testonly = True,
    srcs = [
        "SpoorRuntimeTests.mm",
    ],
    copts = _TEST_COPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime",
    ],
)

objc_library(
    name = "runtime_stub_test_lib",
    testonly = True,
    srcs = [
        "SpoorRuntimeStubTests.mm",
    ],
    copts = _TEST_COPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_stub",
    ],
)

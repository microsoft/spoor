# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "objc_library")
load("@build_bazel_rules_apple//apple:ios.bzl", "ios_static_framework", "ios_unit_test")
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
    "SpoorRuntimeLogger.h",
    "SpoorTypes.h",
]

_LIB_SRCS = [
    "SpoorConfig.mm",
    "SpoorConfig_private.h",
    "SpoorDeletedFilesInfo.mm",
    "SpoorDeletedFilesInfo_private.h",
    "SpoorRuntimeLogger.mm",
]

_WRAPPER_TEST_SRCS = [
    "SpoorConfigTests.mm",
    "SpoorDeletedFilesInfoTests.mm",
    "SpoorTypesTests.mm",
]

_MODULE_NAME = "SpoorRuntime"

ios_static_framework(
    name = "runtime_framework",
    hdrs = _LIB_HDRS,
    minimum_os_version = _MINIMUM_IOS_VERSION,
    bundle_name = _MODULE_NAME,
    visibility = ["//visibility:public"],
    deps = [
        ":runtime_lib",
    ],
)

ios_static_framework(
    name = "runtime_stub_framework",
    hdrs = _LIB_HDRS,
    minimum_os_version = _MINIMUM_IOS_VERSION,
    bundle_name = _MODULE_NAME,
    visibility = ["//visibility:public"],
    deps = [
        ":runtime_stub_lib",
    ],
)

objc_library(
    name = "runtime_lib",
    srcs = _LIB_SRCS,
    hdrs = _LIB_HDRS,
    copts = _LIB_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/runtime",
    ],
)

objc_library(
    name = "runtime_stub_lib",
    srcs = _LIB_SRCS,
    hdrs = _LIB_HDRS,
    copts = _LIB_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/runtime:runtime_stub",
    ],
)

# Tests

ios_unit_test(
    name = "ios_runtime_test",
    minimum_os_version = _MINIMUM_IOS_VERSION,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_test_lib",
        ":runtime_wrapper_test_lib",
    ],
)

macos_unit_test(
    name = "macos_runtime_test",
    minimum_os_version = _MINIMUM_MACOS_VERSION,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_test_lib",
        ":runtime_wrapper_test_lib",
    ],
)

ios_unit_test(
    name = "ios_runtime_stub_test",
    minimum_os_version = _MINIMUM_IOS_VERSION,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_stub_test_lib",
        ":runtime_stub_wrapper_test_lib",
    ],
)

macos_unit_test(
    name = "macos_runtime_stub_test",
    minimum_os_version = _MINIMUM_MACOS_VERSION,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_stub_test_lib",
        ":runtime_stub_wrapper_test_lib",
    ],
)

objc_library(
    name = "runtime_wrapper_test_lib",
    testonly = True,
    srcs = _WRAPPER_TEST_SRCS,
    copts = _TEST_COPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_lib",
    ],
)

objc_library(
    name = "runtime_stub_wrapper_test_lib",
    testonly = True,
    srcs = _WRAPPER_TEST_SRCS,
    copts = _TEST_COPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_stub_lib",
    ],
)

objc_library(
    name = "runtime_test_lib",
    testonly = True,
    srcs = [
        "SpoorRuntimeLoggerTests.mm",
    ],
    copts = _TEST_COPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_lib",
    ],
)

objc_library(
    name = "runtime_stub_test_lib",
    testonly = True,
    srcs = [
        "SpoorRuntimeLoggerStubTests.mm",
    ],
    copts = _TEST_COPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_stub_lib",
    ],
)

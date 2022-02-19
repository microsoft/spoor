# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "objc_library")
load(
    "@build_bazel_rules_apple//apple:ios.bzl",
    "ios_static_framework",
    "ios_unit_test",
)
load("@build_bazel_rules_apple//apple:macos.bzl", "macos_unit_test")
load(
    "//toolchain:config.bzl",
    "SPOOR_DEFAULT_COPTS",
    "SPOOR_DEFAULT_LINKOPTS",
    "SPOOR_DEFAULT_TEST_COPTS",
)

_MINIMUM_IOS_VERSION = "13.0"

_MINIMUM_MACOS_VERSION = "10.15"

_LIB_COPTS = SPOOR_DEFAULT_COPTS

_TEST_COPTS = SPOOR_DEFAULT_TEST_COPTS + [
    "-Wno-gnu-statement-expression",
]

_LIB_HDRS = [
    "SpoorConfig.h",
    "SpoorDeletedFilesInfo.h",
    "Runtime.h",
    "SpoorTypes.h",
]

_LIB_SRCS = [
    "SpoorConfig.mm",
    "SpoorConfig_private.h",
    "SpoorDeletedFilesInfo.mm",
    "SpoorDeletedFilesInfo_private.h",
    "Runtime.mm",
]

_WRAPPER_TEST_SRCS = [
    "SpoorConfigTests.mm",
    "SpoorDeletedFilesInfoTests.mm",
    "SpoorTypesTests.mm",
]

_MODULE_NAME = "SpoorRuntime"

_STUB_MODULE_NAME = "SpoorRuntimeStub"

ios_static_framework(
    name = "ios_runtime_framework",
    hdrs = _LIB_HDRS,
    bundle_name = _MODULE_NAME,
    minimum_os_version = _MINIMUM_IOS_VERSION,
    visibility = ["//visibility:public"],
    deps = [":runtime_lib"],
)

ios_static_framework(
    name = "ios_runtime_stub_framework",
    hdrs = _LIB_HDRS,
    bundle_name = _STUB_MODULE_NAME,
    minimum_os_version = _MINIMUM_IOS_VERSION,
    visibility = ["//visibility:public"],
    deps = [":runtime_stub_lib"],
)

apple_static_library(
    name = "spoor_runtime_ios",
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    minimum_os_version = _MINIMUM_IOS_VERSION,
    platform_type = "ios",
    visibility = ["//visibility:public"],
    deps = [":runtime_lib"],
)

apple_static_library(
    name = "spoor_runtime_macos",
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    minimum_os_version = _MINIMUM_MACOS_VERSION,
    platform_type = "macos",
    visibility = ["//visibility:public"],
    deps = [":runtime_lib"],
)

apple_static_library(
    name = "spoor_runtime_stub_ios",
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    minimum_os_version = _MINIMUM_IOS_VERSION,
    platform_type = "ios",
    visibility = ["//visibility:public"],
    deps = [":runtime_stub_lib"],
)

apple_static_library(
    name = "spoor_runtime_stub_macos",
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    minimum_os_version = _MINIMUM_MACOS_VERSION,
    platform_type = "macos",
    visibility = ["//visibility:public"],
    deps = [":runtime_stub_lib"],
)

apple_static_library(
    name = "spoor_runtime_default_config_ios",
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    minimum_os_version = _MINIMUM_IOS_VERSION,
    platform_type = "ios",
    visibility = ["//visibility:public"],
    deps = [":runtime_default_config_lib"],
)

apple_static_library(
    name = "spoor_runtime_default_config_macos",
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    minimum_os_version = _MINIMUM_IOS_VERSION,
    platform_type = "macos",
    visibility = ["//visibility:public"],
    deps = [":runtime_default_config_lib"],
)

objc_library(
    name = "runtime_lib",
    srcs = _LIB_SRCS,
    hdrs = _LIB_HDRS,
    copts = _LIB_COPTS,
    module_name = _MODULE_NAME,
    visibility = ["//visibility:private"],
    deps = ["//spoor/runtime"],
)

objc_library(
    name = "runtime_stub_lib",
    srcs = _LIB_SRCS,
    hdrs = _LIB_HDRS,
    copts = _LIB_COPTS,
    module_name = _STUB_MODULE_NAME,
    visibility = ["//visibility:private"],
    deps = ["//spoor/runtime:runtime_stub"],
)

objc_library(
    name = "runtime_default_config_lib",
    copts = _LIB_COPTS,
    module_name = _MODULE_NAME,
    visibility = ["//visibility:private"],
    deps = ["//spoor/runtime:runtime_default_config"],
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
        "RuntimeTests.mm",
    ],
    copts = _TEST_COPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_lib",
        "//spoor/runtime:runtime_default_config",
    ],
)

objc_library(
    name = "runtime_stub_test_lib",
    testonly = True,
    srcs = [
        "RuntimeStubTests.mm",
    ],
    copts = _TEST_COPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_stub_lib",
        "//spoor/runtime:runtime_default_config",
    ],
)

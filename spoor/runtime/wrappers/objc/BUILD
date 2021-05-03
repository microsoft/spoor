# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "objc_library")
load("@build_bazel_rules_apple//apple:ios.bzl", "ios_unit_test")
load("@build_bazel_rules_apple//apple:macos.bzl", "macos_unit_test")

_minimum_ios_version = "9.0"
_minimum_macos_version = "10.9"

objc_library(
    name = "runtime_stub",
    module_name = "SpoorRuntime",
    hdrs = [
        "SpoorTypes.h",
        "SpoorConfig.h",
        "SpoorDeletedFilesInfo.h",
        "SpoorRuntime.h",
    ],
    srcs = [
        "SpoorDeletedFilesInfo_private.h",
        "SpoorConfig_private.h",
        "SpoorConfig.mm",
        "SpoorDeletedFilesInfo.mm",
        "SpoorRuntime.mm",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/runtime:runtime_stub",
    ],
)

# Tests

ios_unit_test(
    name = "ios_runtime_wrapper_test",
    minimum_os_version = _minimum_ios_version,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_wrapper_test_lib",
    ],
)

macos_unit_test(
    name = "macos_runtime_wrapper_test",
    minimum_os_version = _minimum_macos_version,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_wrapper_test_lib",
    ],
)

ios_unit_test(
    name = "ios_runtime_stub_test",
    minimum_os_version = _minimum_ios_version,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_stub_test_lib",
    ],
)

macos_unit_test(
    name = "macos_runtime_stub_test",
    minimum_os_version = _minimum_macos_version,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_stub_test_lib",
    ],
)

objc_library(
    name = "runtime_wrapper_test_lib",
    testonly = True,
    copts = ["-Werror", "-Wno-gnu-statement-expression"],
    srcs = [
        "SpoorDeletedFilesInfoTests.mm",
        "SpoorConfigTests.mm",
        "SpoorTypesTests.mm",
    ],
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_stub",
    ],
)

objc_library(
    name = "runtime_stub_test_lib",
    testonly = True,
    copts = ["-Werror", "-Wno-gnu-statement-expression"],
    srcs = [
        "SpoorRuntimeStubTests.mm",
    ],
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_stub",
    ],
)

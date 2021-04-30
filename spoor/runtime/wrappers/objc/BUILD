# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "objc_library")
load("@build_bazel_rules_apple//apple:ios.bzl", "ios_unit_test")

objc_library(
    name = "objc_runtime_stub",
    module_name = "Spoor",
    hdrs = [
        "SpoorTypes.h",
        "SpoorSizeTypeWrapper.h",
        "SpoorConfig.h",
        "SpoorDeletedFilesInfo.h",
        "SpoorTraceFiles.h",
        "SpoorRuntime.h",
    ],
    srcs = [
        "SpoorSizeTypeWrapper_private.h",
        "SpoorDeletedFilesInfo_private.h",
        "SpoorTraceFiles_private.h",
        "SpoorConfig_private.h",
        "SpoorSizeTypeWrapper.mm",
        "SpoorConfig.mm",
        "SpoorDeletedFilesInfo.mm",
        "SpoorTraceFiles.mm",
        "SpoorRuntime.mm",
    ],
    deps = [
        "//spoor/runtime:runtime_stub",
    ],
)

# Tests

ios_unit_test(
    name = "objc_runtime_wrapper_tests",
    minimum_os_version = "9.0",
    deps = [
        ":objc_runtime_wrapper_tests_lib",
    ],
)

ios_unit_test(
    name = "objc_runtime_stub_tests",
    minimum_os_version = "9.0",
    deps = [
        ":objc_runtime_stub_tests_lib",
    ],
)

objc_library(
    name = "objc_runtime_wrapper_tests_lib",
    testonly = 1,
    srcs = [
        "SpoorTraceFilesTests.mm",
        "SpoorDeletedFilesInfoTests.mm",
        "SpoorConfigTests.mm",
        "SpoorSizeTypeWrapperTests.mm",
    ],
    deps = [
        ":objc_runtime_stub",
    ],
)

objc_library(
    name = "objc_runtime_stub_tests_lib",
    testonly = 1,
    srcs = [
        "SpoorRuntimeStubTests.mm",
    ],
    deps = [
        ":objc_runtime_stub",
    ],
)

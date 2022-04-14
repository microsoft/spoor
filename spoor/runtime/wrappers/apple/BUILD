# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "objc_library")
load(
    "@build_bazel_rules_apple//apple:ios.bzl",
    "ios_static_framework",
    "ios_unit_test",
)
load("@build_bazel_rules_apple//apple:apple.bzl", "apple_xcframework")
load("@build_bazel_rules_apple//apple:macos.bzl", "macos_unit_test")
load(
    "@build_bazel_rules_apple//apple:watchos.bzl",
    "watchos_static_framework",
    # TODO(237): Fix watchOS unit tests.
    # "watchos_unit_test",
)
load(
    "//spoor/runtime/wrappers/apple:config.bzl",
    "RUNTIME_HDRS",
    "RUNTIME_STUB_TARGET_NAME",
    "RUNTIME_TARGET_NAME",
)
load(
    "//toolchain:config.bzl",
    "SPOOR_DEFAULT_COPTS",
    "SPOOR_DEFAULT_LINKOPTS",
    "SPOOR_DEFAULT_TEST_COPTS",
)

_MINIMUM_IOS_VERSION = "13.0"

_MINIMUM_MACOS_VERSION = "10.15"

_MINIMUM_WATCHOS_VERSION = "6.0"

_LIB_COPTS = SPOOR_DEFAULT_COPTS

_TEST_COPTS = SPOOR_DEFAULT_TEST_COPTS + ["-Wno-gnu-statement-expression"]

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

_MODULE_NAME = RUNTIME_TARGET_NAME

_STUB_MODULE_NAME = RUNTIME_STUB_TARGET_NAME

ios_static_framework(
    name = "ios_runtime_framework",
    hdrs = RUNTIME_HDRS,
    bundle_name = _MODULE_NAME,
    minimum_os_version = _MINIMUM_IOS_VERSION,
    visibility = ["//visibility:public"],
    deps = [":runtime_lib"],
)

_XCFRAMEWORK_BUNDLE_ID_TEMPLATE = "com.microsoft.spoor.{}"

apple_xcframework(
    name = RUNTIME_TARGET_NAME,
    bundle_id = _XCFRAMEWORK_BUNDLE_ID_TEMPLATE.format(RUNTIME_TARGET_NAME),
    families_required = {
        "ios": [
            "ipad",
            "iphone",
        ],
    },
    framework_type = ["dynamic"],
    infoplists = ["SpoorRuntimeInfo.plist"],
    ios = {
        "simulator": [
            "arm64",
            "x86_64",
        ],
        "device": ["arm64"],
    },
    minimum_deployment_os_versions = {
        "ios": _MINIMUM_IOS_VERSION,
    },
    minimum_os_versions = {
        "ios": _MINIMUM_IOS_VERSION,
    },
    public_hdrs = RUNTIME_HDRS,
    visibility = ["//visibility:public"],
    deps = [
        ":runtime_default_config_lib",
        ":runtime_lib",
    ],
)

ios_static_framework(
    name = "ios_runtime_stub_framework",
    hdrs = RUNTIME_HDRS,
    bundle_name = _STUB_MODULE_NAME,
    minimum_os_version = _MINIMUM_IOS_VERSION,
    visibility = ["//visibility:public"],
    deps = [":runtime_stub_lib"],
)

apple_xcframework(
    name = RUNTIME_STUB_TARGET_NAME,
    bundle_id = _XCFRAMEWORK_BUNDLE_ID_TEMPLATE.format(
        RUNTIME_STUB_TARGET_NAME,
    ),
    families_required = {
        "ios": [
            "ipad",
            "iphone",
        ],
    },
    framework_type = ["dynamic"],
    infoplists = ["SpoorRuntimeInfo.plist"],
    ios = {
        "simulator": [
            "arm64",
            "x86_64",
        ],
        "device": ["arm64"],
    },
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    minimum_deployment_os_versions = {
        "ios": _MINIMUM_IOS_VERSION,
    },
    minimum_os_versions = {
        "ios": _MINIMUM_IOS_VERSION,
    },
    public_hdrs = RUNTIME_HDRS,
    visibility = ["//visibility:public"],
    deps = [
        ":runtime_default_config_lib",
        ":runtime_stub_lib",
    ],
)

watchos_static_framework(
    name = "watchos_runtime_framework",
    hdrs = RUNTIME_HDRS,
    bundle_name = _MODULE_NAME,
    minimum_os_version = _MINIMUM_WATCHOS_VERSION,
    visibility = ["//visibility:public"],
    deps = [":runtime_lib"],
)

watchos_static_framework(
    name = "watchos_runtime_stub_framework",
    hdrs = RUNTIME_HDRS,
    bundle_name = _STUB_MODULE_NAME,
    minimum_os_version = _MINIMUM_WATCHOS_VERSION,
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
    name = "spoor_runtime_watchos",
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    minimum_os_version = _MINIMUM_WATCHOS_VERSION,
    platform_type = "watchos",
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
    name = "spoor_runtime_stub_watchos",
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    minimum_os_version = _MINIMUM_WATCHOS_VERSION,
    platform_type = "watchos",
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

apple_static_library(
    name = "spoor_runtime_default_config_watchos",
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    minimum_os_version = _MINIMUM_WATCHOS_VERSION,
    platform_type = "watchos",
    visibility = ["//visibility:public"],
    deps = [":runtime_default_config_lib"],
)

objc_library(
    name = "runtime_lib",
    srcs = _LIB_SRCS,
    hdrs = RUNTIME_HDRS,
    copts = _LIB_COPTS,
    module_name = _MODULE_NAME,
    visibility = ["//visibility:private"],
    deps = ["//spoor/runtime"],
)

objc_library(
    name = "runtime_stub_lib",
    srcs = _LIB_SRCS,
    hdrs = RUNTIME_HDRS,
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

# TODO(237): Fix watchOS unit tests.
# watchos_unit_test(
#     name = "watchos_runtime_test",
#     minimum_os_version = _MINIMUM_WATCHOS_VERSION,
#     visibility = ["//visibility:private"],
#     deps = [
#         ":runtime_test_lib",
#         ":runtime_wrapper_test_lib",
#     ],
# )

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

# TODO(237): Fix watchOS unit tests.
# watchos_unit_test(
#     name = "watchos_runtime_stub_test",
#     minimum_os_version = _MINIMUM_WATCHOS_VERSION,
#     visibility = ["//visibility:private"],
#     deps = [
#         ":runtime_stub_test_lib",
#         ":runtime_stub_wrapper_test_lib",
#     ],
# )

objc_library(
    name = "runtime_wrapper_test_lib",
    testonly = True,
    srcs = _WRAPPER_TEST_SRCS,
    copts = _TEST_COPTS,
    visibility = ["//visibility:private"],
    deps = [":runtime_lib"],
)

objc_library(
    name = "runtime_stub_wrapper_test_lib",
    testonly = True,
    srcs = _WRAPPER_TEST_SRCS,
    copts = _TEST_COPTS,
    visibility = ["//visibility:private"],
    deps = [":runtime_stub_lib"],
)

objc_library(
    name = "runtime_test_lib",
    testonly = True,
    srcs = ["RuntimeTests.mm"],
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
    srcs = ["RuntimeStubTests.mm"],
    copts = _TEST_COPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_stub_lib",
        "//spoor/runtime:runtime_default_config",
    ],
)

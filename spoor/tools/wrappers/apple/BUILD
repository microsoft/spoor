# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load(
    "@build_bazel_rules_apple//apple:macos.bzl",
    "macos_command_line_application",
)
load("//toolchain:config.bzl", "SPOOR_DEFAULT_LINKOPTS")

_MINIMUM_MACOS_VERSION = "10.15"

macos_command_line_application(
    name = "spoor",
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    minimum_deployment_os_version = _MINIMUM_MACOS_VERSION,
    minimum_os_version = _MINIMUM_MACOS_VERSION,
    visibility = ["//visibility:public"],
    deps = ["//spoor/tools:spoor_lib"],
)

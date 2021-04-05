# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "objc_library")
load("//toolchain:cc_static_library.bzl", "cc_static_library")

objc_library(
    name = "runtime",
    deps = ["//spoor/runtime:runtime"],
    toolchains = ["//toolchain/crosstool:llvm_toolchain"]
)

apple_static_library(
    name = "SpoorRuntime",
    minimum_os_version = "9.0",
    platform_type = "ios",
    deps = [":runtime"],
)

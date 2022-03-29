# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library")
load("//toolchain:config.bzl", "SPOOR_DEFAULT_COPTS", "SPOOR_DEFAULT_LINKOPTS")

# spoor::common::util::ValueFromSourceOrDefault is tested in the instrumentation
# and runtime configs.
cc_library(
    name = "util",
    hdrs = ["util.h"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//spoor:__subpackages__"],
)

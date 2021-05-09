# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_binary")
load("//toolchain:config.bzl", "SPOOR_DEFAULT_COPTS", "SPOOR_DEFAULT_LINKOPTS")

filegroup(
    name = "copyright_header",
    srcs = ["copyright_header.txt"],
    visibility = ["//visibility:private"],
)

cc_binary(
    name = "add_copyright_header",
    srcs = ["add_copyright_header.cc"],
    copts = SPOOR_DEFAULT_COPTS,
    data = [":copyright_header"],
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//util:numeric",
        "//util:result",
    ],
)

# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_binary")

filegroup(
    name = "copyright_header",
    srcs = ["copyright_header.txt"],
    visibility = ["//visibility:private"],
)

cc_binary(
    name = "add_copyright_header",
    srcs = ["add_copyright_header.cc"],
    copts = ["-Werror"],
    data = [":copyright_header"],
    visibility = ["//visibility:public"],
    deps = [
        "//util:numeric",
        "//util:result",
    ],
)

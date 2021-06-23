# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "tomlplusplus",
    hdrs = ["toml.h"],
    include_prefix = "tomlplusplus",
    visibility = ["//visibility:public"],
)

genrule(
    name = "toml_h",
    srcs = ["toml.hpp"],
    outs = ["toml.h"],
    cmd = "cp $< $@",
    visibility = ["//visibility:private"],
)

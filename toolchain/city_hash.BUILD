# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "city_hash",
    srcs = [
        "config.h",
        "src/city.h",
        "src/city_patch.cc",
    ],
    hdrs = ["src/city.h"],
    include_prefix = "city_hash",
    strip_include_prefix = "src",
    visibility = ["//visibility:public"],
)

genrule(
    name = "city_cc",
    srcs = ["src/city.cc"],
    outs = ["src/city_patch.cc"],
    cmd = "sed 's|#include <city.h>|#include \"src/city.h\"|g' $< >$@",
    visibility = ["//visibility:private"],
)

genrule(
    name = "config_h",
    outs = ["config.h"],
    cmd = "touch $@",
    visibility = ["//visibility:private"],
)

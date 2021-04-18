# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "gsl",
    hdrs = glob(["include/gsl/*"]),
    copts = ["-Werror"],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)

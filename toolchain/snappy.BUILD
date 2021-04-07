# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "snappy",
    srcs = [
        "snappy.cc",
        "snappy.h",
        "snappy-internal.h",
        "snappy-sinksource.cc",
        "snappy-sinksource.h",
        "snappy-stubs-internal.cc",
        "snappy-stubs-internal.h",
        "snappy-stubs-public.h",
    ],
    hdrs = ["snappy.h"],
    include_prefix = "snappy",
    visibility = ["//visibility:public"],
)

genrule(
    name = "snappy_stubs_public_h",
    srcs = ["snappy-stubs-public.h.in"],
    outs = ["snappy-stubs-public.h"],
    cmd = "sed " + " ".join([
        "-e 's/$${HAVE_SYS_UIO_H_01}/1/g'",
        "-e 's/$${PROJECT_VERSION_MAJOR}/'$$(sed -En 's/^.*VERSION ([0-9]+).[0-9]+.[0-9]+.*$$/\1/p' CMakeLists.txt)'/g'",
        "-e 's/$${PROJECT_VERSION_MINOR}/'$$(sed -En 's/^.*VERSION [0-9]+.([0-9]+).[0-9]+.*$$/\1/p' CMakeLists.txt)'/g'",
        "-e 's/$${PROJECT_VERSION_PATCH}/'$$(sed -En 's/^.*VERSION [0-9]+.[0-9]+.([0-9]+).*$$/\1/p' CMakeLists.txt)'/g'",
    ]) + " $< >$@",
    visibility = ["//visibility:private"],
)

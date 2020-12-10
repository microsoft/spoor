# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_proto_library", "cc_test")
load("@rules_proto//proto:defs.bzl", "proto_library")

proto_library(
    name = "instrumentation_map_proto",
    srcs = ["instrumentation_map.proto"],
    visibility = ["//visibility:public"],
    deps = ["@com_google_protobuf//:timestamp_proto"],
)

cc_proto_library(
    name = "instrumentation_map_cc_proto",
    visibility = ["//visibility:public"],
    deps = [":instrumentation_map_proto"],
)

# Hack to produce a shared library with the correct extension for each platform.

SHARED_LIBRARY_NAME = "spoor_instrumentation"

SHARED_LIBRARY_SRCS = [
    "instrumentation.h",
    "register_pass.cc",
]

SHARED_LIBRARY_COPTS = ["-Werror"]

SHARED_LIBRARY_LINKSHARED = True

SHARED_LIBRARY_VISIBILITY = ["//visibility:public"]

SHARED_LIBRARY_DEPS = [
    "//spoor/instrumentation/config",
    "//spoor/instrumentation/inject_runtime",
    "@llvm//11.0.0",
]

cc_binary(
    name = "lib" + SHARED_LIBRARY_NAME + ".so",
    srcs = SHARED_LIBRARY_SRCS,
    copts = SHARED_LIBRARY_COPTS,
    linkshared = SHARED_LIBRARY_LINKSHARED,
    visibility = SHARED_LIBRARY_VISIBILITY,
    deps = SHARED_LIBRARY_DEPS,
)

cc_binary(
    name = "lib" + SHARED_LIBRARY_NAME + ".dylib",
    srcs = SHARED_LIBRARY_SRCS,
    copts = SHARED_LIBRARY_COPTS,
    linkshared = SHARED_LIBRARY_LINKSHARED,
    visibility = SHARED_LIBRARY_VISIBILITY,
    deps = SHARED_LIBRARY_DEPS,
)

sh_test(
    name = "register_pass_test",
    srcs = ["register_pass_test.sh"],
    data = [
        ":libspoor_instrumentation.dylib",
        ":libspoor_instrumentation.so",
        "//spoor/instrumentation/test_data",
    ],
    visibility = ["//visibility:private"],
)

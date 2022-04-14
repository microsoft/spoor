# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")
load(
    "//toolchain:config.bzl",
    "SPOOR_DEFAULT_COPTS",
    "SPOOR_DEFAULT_LINKOPTS",
    "SPOOR_DEFAULT_TEST_COPTS",
    "SPOOR_DEFAULT_TEST_LINKOPTS",
)

cc_library(
    name = "instrumentation_info",
    hdrs = ["instrumentation.h"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:public"],
    deps = ["//util:numeric"],
)

cc_test(
    name = "instrumentation_info_test",
    size = "small",
    srcs = ["instrumentation_test.cc"],
    copts = SPOOR_DEFAULT_TEST_COPTS,
    linkopts = SPOOR_DEFAULT_TEST_LINKOPTS,
    visibility = ["//visibility:private"],
    deps = [
        ":instrumentation_info",
        "//spoor/instrumentation/symbols:symbols_cc_proto",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "spoor_opt_lib",
    srcs = ["spoor_opt.cc"],
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = [
        "//spoor/instrumentation:__pkg__",
        "//spoor/instrumentation/wrappers/apple:__pkg__",
    ],
    deps = [
        ":instrumentation_info",
        "//spoor/instrumentation/config",
        "//spoor/instrumentation/config:source",
        "//spoor/instrumentation/filters:filters_reader",
        "//spoor/instrumentation/inject_instrumentation",
        "//spoor/instrumentation/symbols:symbols_writer",
        "//util/file_system",
        "//util/time:clock",
        "@com_google_absl//absl/flags:config",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
        "@com_microsoft_gsl//:gsl",
        "@llvm-project//llvm:BitWriter",
        "@llvm-project//llvm:Core",
        "@llvm-project//llvm:Passes",
        "@llvm-project//llvm:Support",
        "@llvm-project//llvm:config",
    ],
)

cc_binary(
    name = "spoor_opt",
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    visibility = ["//visibility:public"],
    deps = [":spoor_opt_lib"],
)

# TODO(#131): Fix `opt` pass plugin support for IR instrumentation. Spoor pass
# plugins are broken (see ticket) but are still built to prevent further
# regression.

# Hack to produce a shared library with the correct extension for each platform.

SHARED_LIBRARY_NAME = "spoor_instrumentation"

SHARED_LIBRARY_SRCS = ["spoor_instrumentation_pass.cc"]

SHARED_LIBRARY_LINKSHARED = True

SHARED_LIBRARY_VISIBILITY = ["//visibility:public"]

SHARED_LIBRARY_DEPS = [
    ":instrumentation_info",
    "//spoor/instrumentation/config",
    "//spoor/instrumentation/config:source",
    "//spoor/instrumentation/config:output_language",
    "//spoor/instrumentation/inject_instrumentation",
    "//spoor/instrumentation/symbols:symbols_writer",
    "//util/file_system",
    "//util/time:clock",
    "@com_google_absl//absl/strings:str_format",
    "@com_google_protobuf//:protobuf",
    "@llvm-project//llvm:Support",
    "@llvm-project//llvm:Core",
    "@llvm-project//llvm:Passes",
]

cc_binary(
    name = "lib" + SHARED_LIBRARY_NAME + ".so",
    srcs = SHARED_LIBRARY_SRCS,
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    linkshared = SHARED_LIBRARY_LINKSHARED,
    visibility = SHARED_LIBRARY_VISIBILITY,
    deps = SHARED_LIBRARY_DEPS,
)

cc_binary(
    name = "lib" + SHARED_LIBRARY_NAME + ".dylib",
    srcs = SHARED_LIBRARY_SRCS,
    copts = SPOOR_DEFAULT_COPTS,
    linkopts = SPOOR_DEFAULT_LINKOPTS,
    linkshared = SHARED_LIBRARY_LINKSHARED,
    visibility = SHARED_LIBRARY_VISIBILITY,
    deps = SHARED_LIBRARY_DEPS,
)

sh_test(
    name = "spoor_opt_test",
    size = "small",
    srcs = ["spoor_opt_test.sh"],
    data = [
        ":spoor_opt",
        "//spoor/instrumentation/test_data",
    ],
    visibility = ["//visibility:private"],
)

sh_test(
    name = "register_pass_test",
    size = "small",
    srcs = ["register_pass_test.sh"],
    data = [
        ":libspoor_instrumentation.dylib",
        ":libspoor_instrumentation.so",
        "//spoor/instrumentation/test_data",
    ],
    visibility = ["//visibility:private"],
)

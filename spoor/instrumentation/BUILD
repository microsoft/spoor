# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_test")

cc_binary(
    name = "spoor_opt",
    srcs = [
        "instrumentation.h",
        "spoor_opt.cc",
    ],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/instrumentation/config",
        "//spoor/instrumentation/config:command_line_config",
        "//spoor/instrumentation/inject_instrumentation",
        "//spoor/instrumentation/support",
        "//util/time:clock",
        "@com_google_absl//absl/flags:config",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/strings",
        "@com_microsoft_gsl//:gsl",
        "@llvm//12.0.0",
    ],
)

# Hack to produce a shared library with the correct extension for each platform.

SHARED_LIBRARY_NAME = "spoor_instrumentation"

SHARED_LIBRARY_SRCS = [
    "instrumentation.h",
    "spoor_instrumentation_pass.cc",
]

SHARED_LIBRARY_COPTS = ["-Werror"]

SHARED_LIBRARY_LINKSHARED = True

SHARED_LIBRARY_VISIBILITY = ["//visibility:public"]

SHARED_LIBRARY_DEPS = [
    "//spoor/instrumentation/config:env_config",
    "//spoor/instrumentation/inject_instrumentation",
    "//spoor/instrumentation/support",
    "//util/time:clock",
    "@com_google_absl//absl/strings:str_format",
    "@llvm//12.0.0",
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

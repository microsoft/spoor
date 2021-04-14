# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_binary(
    name = "spoor",
    srcs = ["spoor.cc"],
    copts = ["-Werror"],
    deps = [
        "//spoor/proto:perfetto_trace_cc_proto",
        "//spoor/proto:spoor_cc_proto",
        "//spoor/runtime/trace",
        "//util:numeric",
        "//util/env",
        "//util/compression",
        "@com_microsoft_gsl//:gsl",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)

# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

proto_library(
    name = "spoor_proto",
    srcs = ["spoor.proto"],
    visibility = ["//visibility:public"],
    deps = ["@com_google_protobuf//:timestamp_proto"],
)

cc_proto_library(
    name = "spoor_cc_proto",
    visibility = ["//visibility:public"],
    deps = [":spoor_proto"],
)

cc_proto_library(
    name = "perfetto_trace_cc_proto",
    visibility = ["//visibility:public"],
    deps = ["@perfetto//:protos_perfetto_trace_non_minimal_protos"],
)


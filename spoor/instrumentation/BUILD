load("@rules_cc//cc:defs.bzl", "cc_library", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

proto_library(
    name = "instrumentation_map_proto",
    srcs = ["instrumentation_map.proto"],
    visibility = ["//visibility:public"],
)

cc_proto_library(
    name = "instrumentation_map_cc_proto",
    deps = [":instrumentation_map_proto"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "instrumentation",
    hdrs = ["instrumentation.h"],
    srcs = ["instrumentation.cc"],
    copts = [
        "-Wno-unused-parameter",  # Since we're including LLVM in this module.
        "-Werror",
        "-I/usr/local/opt/llvm/include",
        "-std=c++17",  # LLVM 10 does not support C++20.
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/instrumentation/config:config",
        "//spoor/instrumentation/inject_runtime:inject_runtime",
        #"@org_llvm_llvm//:llvm"
    ],
)

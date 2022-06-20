# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "demangle",
    srcs = glob([
        "lib/Demangling/**/*.cpp",
        "lib/Demangling/**/*.h",
        "stdlib/public/**/*.h",
        "swift/lib/Demangling/**/*.cpp",
        "swift/lib/SwiftDemangle/**/*.cpp",
        "swift/lib/SwiftDemangle/**/*.h",
    ]),
    hdrs = glob([
        "include/swift/**/*.h",
        "include/swift/**/*.def",
    ]),
    copts = [
        "-std=c++14",
        "-DSWIFT_STDLIB_HAS_TYPE_PRINTING=1",
    ],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
    deps = [
        "@llvm-project//llvm:Demangle",
        "@llvm-project//llvm:Support",
    ],
)

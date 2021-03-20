# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cc_library(
    name = "file_system",
    srcs = [
        "local_file_system.cc",
        "local_file_writer.cc",
    ],
    hdrs = [
        "file_system.h",
        "file_writer.h",
        "local_file_system.h",
        "local_file_writer.h",
    ],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "//util:result",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_library(
    name = "file_system_mock",
    srcs = ["directory_entry_mock.cc"],
    hdrs = [
        "directory_entry_mock.h",
        "file_system_mock.h",
        "file_writer_mock.h",
    ],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        ":file_system",
        "//util:result",
        "@com_google_googletest//:gtest",
        "@com_microsoft_gsl//:gsl",
    ],
)

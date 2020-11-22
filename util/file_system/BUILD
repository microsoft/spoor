cc_library(
    name = "file_system",
    srcs = ["local_file_system.cc"],
    hdrs = [
        "file_system.h",
        "local_file_system.h",
    ],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "file_system_mock",
    srcs = ["directory_entry_mock.cc"],
    hdrs = [
        "directory_entry_mock.h",
        "file_system_mock.h",
    ],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        ":file_system",
        "@com_google_googletest//:gtest",
    ],
)

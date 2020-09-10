load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "buffer",
    hdrs = [
        "amalgamated_buffer_slice_pool.h",
        "buffer_slice.h",
        "buffer_slice_pool.h",
        "circular_slice_buffer.h",
        "contiguous_memory.h",
        "dynamic_buffer_slice_pool.h",
        "reserved_buffer_slice_pool.h",
    ],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "//util/memory",
        "//util:numeric",
        "//util:result",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "buffer_slice_test",
    size = "small",
    srcs = ["buffer_slice_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
        "//util/memory",
        "//util:numeric",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "dynamic_buffer_slice_pool_test",
    size = "small",
    srcs = ["dynamic_buffer_slice_pool_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
        "//util/memory",
        "//util:numeric",
        "//util:result",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "reserved_buffer_slice_pool_test",
    size = "small",
    srcs = ["reserved_buffer_slice_pool_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
        "//util/memory",
        "//util:numeric",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "buffer_slice_pool_test",
    size = "small",
    srcs = ["buffer_slice_pool_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
        "//util/memory",
        "//util:numeric",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "amalgamated_buffer_slice_pool_test",
    size = "small",
    srcs = ["amalgamated_buffer_slice_pool_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
        "//util/memory",
        "//util:numeric",
        "//util:result",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "circular_slice_buffer_test",
    size = "small",
    srcs = ["circular_slice_buffer_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
        "//util:numeric",
        "@com_google_googletest//:gtest_main",
    ],
)

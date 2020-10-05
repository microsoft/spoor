load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "buffer",
    hdrs = [
        "amalgamated_buffer_slice_pool.h",
        "buffer_slice_pool.h",
        "circular_buffer.h",
        "circular_slice_buffer.h",
        "dynamic_buffer_slice_pool.h",
        "owned_buffer_slice.h",
        "reserved_buffer_slice_pool.h",
        "unowned_buffer_slice.h",
    ],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "//util:numeric",
        "//util:result",
        "//util/memory",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "circular_buffer_test",
    size = "small",
    srcs = ["circular_buffer_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
        "//util:numeric",
        "//util/memory",
        "@com_google_googletest//:gtest_main",
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
        "//util:numeric",
        "//util/memory",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "owned_buffer_slice_test",
    size = "small",
    srcs = ["owned_buffer_slice_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
        "//util:numeric",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "unowned_buffer_slice_test",
    size = "small",
    srcs = ["unowned_buffer_slice_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
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
        "//util:numeric",
        "//util/memory",
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
        "//util:numeric",
        "//util:result",
        "//util/memory",
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
        "//util:numeric",
        "//util/memory",
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
        "//util:numeric",
        "//util:result",
        "//util/memory",
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

cc_binary(
    name = "circular_slice_buffer_benchmark",
    srcs = ["circular_slice_buffer_benchmark.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":buffer",
        "//util:numeric",
        "@com_google_benchmark//:benchmark",
        "@com_microsoft_gsl//:gsl",
    ],
)

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "buffer",
    hdrs = [
        "buffer_slice.h",
        "buffer_slice_pool.h",
        "buffer_slice_pool_ownership_info.h",
        "circular_slice_buffer.h",
        "contiguous_memory.h",
    ],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "//gsl:pointers",
        "//util:memory",
        "//util:numeric",
        "//util:result",
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
        "//util:memory",
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
        "//util:memory",
        "//util:numeric",
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

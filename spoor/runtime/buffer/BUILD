load("@rules_cc//cc:defs.bzl", "cc_library")

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

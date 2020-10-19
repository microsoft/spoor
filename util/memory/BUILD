cc_library(
    name = "memory",
    hdrs = [
        "owned_ptr.h",
        "ptr_owner.h",
    ],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = ["//util:result"],
)

cc_test(
    name = "memory_test",
    size = "small",
    srcs = ["memory_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":memory",
        "//util:numeric",
        "//util:result",
        "@com_google_googletest//:gtest_main",
    ],
)

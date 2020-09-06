cc_library(
    name = "flags",
    srcs = ["flags.cc"],
    hdrs = ["flags.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [],
)

cc_test(
    name = "flags_test",
    size = "small",
    srcs = ["flags_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":flags",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "memory",
    hdrs = ["memory.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = ["//gsl:pointers"],
)

cc_test(
    name = "memory_test",
    size = "small",
    srcs = ["memory_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":result",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "numeric",
    hdrs = ["numeric.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "numeric_test",
    size = "small",
    srcs = ["numeric_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":numeric",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "result",
    hdrs = ["result.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "result_test",
    size = "small",
    srcs = ["result_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        ":numeric",
        ":result",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "strings",
    hdrs = ["strings.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [],
)

cc_test(
    name = "strings_test",
    size = "small",
    srcs = ["strings_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":strings",
        "@com_google_googletest//:gtest_main",
    ],
)

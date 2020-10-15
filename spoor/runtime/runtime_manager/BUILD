load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "runtime_manager",
    srcs = ["runtime_manager.cc"],
    hdrs = ["runtime_manager.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/runtime/buffer:buffer",
        "//spoor/runtime/config:config",
        "//spoor/runtime/event_logger:event_logger",
        "//spoor/runtime/flush_queue:flush_queue",
        "//spoor/runtime/trace:trace",
        "//util:numeric",
        "@com_microsoft_gsl//:gsl",
    ],
)

cc_test(
    name = "runtime_manager_test",
    size = "small",
    srcs = ["runtime_manager_test.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:private"],
    deps = [
        ":runtime_manager",
        "@com_google_googletest//:gtest_main",
    ],
)

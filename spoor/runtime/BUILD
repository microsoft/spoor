load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "runtime",
    srcs = ["runtime.cc"],
    hdrs = ["runtime.h"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/runtime/config:config",
        "//spoor/runtime/event_logger:event_logger",
        "//spoor/runtime/runtime_manager:runtime_manager",
        "//spoor/runtime/trace:trace",
        "//util:numeric",
        "@com_microsoft_gsl//:gsl",
    ],
)

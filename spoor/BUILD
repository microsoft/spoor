load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "bin",
    srcs = ["main.cc"],
    copts = ["-Werror"],
    visibility = ["//visibility:public"],
    deps = [
        "//spoor/runtime:runtime",
        "//util:numeric",
    ],
)

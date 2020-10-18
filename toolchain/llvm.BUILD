load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "llvm",
    hdrs = glob(["llvm/include/**/*.h"]),
    # copts = ["-Werror"],
    strip_include_prefix = "llvm/include",
    visibility = ["//visibility:public"],
)

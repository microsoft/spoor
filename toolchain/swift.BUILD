load("@rules_cc//cc:defs.bzl", "cc_library")

cc_import(
    name = "pass",
    static_library = "libLLVMPasses.a",
)

cc_library(
    name = "demangle",
    srcs = glob([
        "swift/lib/Demangling/**/*.cpp",
        "lib/Demangling/**/*.cpp",
        "lib/Demangling/**/*.h",
    ]),
    hdrs = glob([
        "include/swift/**/*.h",
        "include/swift/**/*.def",
    ]),
    copts = [
        "-Wno-dollar-in-identifier-extension",
        "-Wno-unused-parameter",
        "-Werror"
    ],
    linkopts = [
        "-llibLLVMPasses",
    ],
    deps = [
        # "@llvm_toolchain//:lib",
        # "@org_llvm_llvm//:llvm_passes",
    ],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)

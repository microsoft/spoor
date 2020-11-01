# load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
# 
# cc_library(
#     name = "inject_runtime",
#     srcs = ["inject_runtime.cc"],
#     hdrs = ["inject_runtime.h"],
#     copts = [
#         "-Wno-unused-parameter",  # Since we're including LLVM in this module.
#         "-Wno-unused-command-line-argument",
#         "-Werror",
#         "-std=c++17",  # LLVM 10 does not support C++20.
#     ],
#     visibility = ["//visibility:public"],
#     deps = [
#         "//spoor/instrumentation:instrumentation_map_cc_proto",
#         "//util:numeric",
#         # "@com_apple_swift//:demangle",
#         # "@llvm_toolchain//:lib/libLLVMPasses.a",
#         "@com_google_absl//absl/strings:str_format",
#         "@com_microsoft_gsl//:gsl",
#     ],
# )
# 
# cc_test(
#     name = "inject_runtime_test",
#     size = "small",
#     srcs = ["inject_runtime_test.cc"],
#     copts = [
#         "-Wno-unused-parameter",  # Since we're including LLVM in this module.
#         "-Wno-unused-command-line-argument",
#         "-Werror",
#         "-std=c++17",  # LLVM 10 does not support C++20.
#     ],
#     data = ["//spoor/instrumentation/inject_runtime/test_data"],
#     visibility = ["//visibility:private"],
#     deps = [
#         ":inject_runtime",
#         "//spoor/instrumentation:instrumentation_map_cc_proto",
#         # "@org_llvm_llvm",
#         "@com_google_googletest//:gtest_main",
#         "@com_google_protobuf//:protobuf",
#         "@com_microsoft_gsl//:gsl",
#     ],
# )

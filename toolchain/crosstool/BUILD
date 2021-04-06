# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load(
    ":cc_toolchain_config.bzl",
    "cc_toolchain_config_ios_arm64",
    "cc_toolchain_config_ios_x86_64",
    "cc_toolchain_config_linux",
    "cc_toolchain_config_macos_x86_64",
)

cc_toolchain_suite(
    name = "llvm_toolchain",
    toolchains = {
        "darwin": ":macos_x86_64_llvm_toolchain",
        "darwin_x86_64": ":macos_x86_64_llvm_toolchain",
        "ios_arm64": ":ios_arm64_llvm_toolchain",
        "ios_x86_64": ":ios_x86_64_llvm_toolchain",
        "k8": ":linux_llvm_toolchain",
    },
    visibility = ["//visibility:public"],
)

filegroup(name = "empty")

cc_toolchain(
    name = "ios_arm64_llvm_toolchain",
    all_files = ":empty",
    compiler_files = ":empty",
    dwp_files = ":empty",
    linker_files = ":empty",
    objcopy_files = ":empty",
    strip_files = ":empty",
    supports_param_files = False,
    toolchain_config = ":ios_arm64_llvm_toolchain_config",
    toolchain_identifier = "ios-arm64-llvm-toolchain",
)

cc_toolchain_config_ios_arm64(name = "ios_arm64_llvm_toolchain_config")

cc_toolchain(
    name = "ios_x86_64_llvm_toolchain",
    all_files = ":empty",
    compiler_files = ":empty",
    dwp_files = ":empty",
    linker_files = ":empty",
    objcopy_files = ":empty",
    strip_files = ":empty",
    supports_param_files = False,
    toolchain_config = ":ios_x86_64_llvm_toolchain_config",
    toolchain_identifier = "ios-x86_64-llvm-toolchain",
)

cc_toolchain_config_ios_x86_64(name = "ios_x86_64_llvm_toolchain_config")

cc_toolchain(
    name = "linux_llvm_toolchain",
    all_files = ":empty",
    compiler_files = ":empty",
    dwp_files = ":empty",
    linker_files = ":empty",
    objcopy_files = ":empty",
    strip_files = ":empty",
    supports_param_files = False,
    toolchain_config = ":linux_llvm_toolchain_config",
    toolchain_identifier = "linux-llvm-toolchain",
)

cc_toolchain_config_linux(name = "linux_llvm_toolchain_config")

cc_toolchain(
    name = "macos_x86_64_llvm_toolchain",
    all_files = ":empty",
    compiler_files = ":empty",
    dwp_files = ":empty",
    linker_files = ":empty",
    objcopy_files = ":empty",
    strip_files = ":empty",
    supports_param_files = False,
    toolchain_config = ":macos_x86_64_llvm_toolchain_config",
    toolchain_identifier = "macos-x86_64-llvm-toolchain",
)

cc_toolchain_config_macos_x86_64(name = "macos_x86_64_llvm_toolchain_config")

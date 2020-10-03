load(
    ":cc_toolchain_config.bzl",
    "cc_toolchain_config_darwin",
    "cc_toolchain_config_linux"
)

cc_toolchain_suite(
    name = "llvm_toolchain",
    toolchains = {
        "k8": ":linux_llvm_toolchain",
        "darwin": ":darwin_llvm_toolchain",
    },
    visibility = ["//visibility:public"],
)

filegroup(name = "empty")

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
    name = "darwin_llvm_toolchain",
    all_files = ":empty",
    compiler_files = ":empty",
    dwp_files = ":empty",
    linker_files = ":empty",
    objcopy_files = ":empty",
    strip_files = ":empty",
    supports_param_files = False,
    toolchain_config = ":darwin_llvm_toolchain_config",
    toolchain_identifier = "darwin-llvm-toolchain",
)

cc_toolchain_config_darwin(name = "darwin_llvm_toolchain_config")

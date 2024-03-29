# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")
load(
    "@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
    "feature",
    "feature_set",
    "flag_group",
    "flag_set",
    "tool_path",
)

# Tip: Determine the compiler's include paths with the incantation
# `clang++ -E -xc++ - -v`.

FEATURES = [
    feature(
        name = "default_linker_flags",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = [ACTION_NAMES.cpp_link_executable],
                flag_groups = ([
                    flag_group(
                        flags = [
                            "-lc++",
                            "-lm",
                            "-lpthread",
                            "-lz",
                        ],
                    ),
                ]),
            ),
        ],
    ),
    feature(
        name = "default_compiler_flags",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = [ACTION_NAMES.cpp_compile],
                flag_groups = ([
                    flag_group(
                        flags = [
                            "-std=c++20",
                            "-pthread",
                            "-fno-exceptions",
                            "-fno-rtti",
                            "-Wall",
                            "-Wextra",
                            "-pedantic",
                            "-Wno-gnu-zero-variadic-macro-arguments",  # gMock
                            "-Wno-gcc-compat",  # absl::StrFormat.
                        ],
                    ),
                ]),
            ),
        ],
    ),
]

def linux_llvm_toolchain_impl(ctx):
    # Default paths on Linux when installing LLVM tools via apt.
    tool_paths = [
        tool_path(
            name = "gcc",
            path = "/usr/bin/clang-13",
        ),
        tool_path(
            name = "ld",
            path = "/usr/bin/lld-13",
        ),
        tool_path(
            name = "ar",
            path = "/usr/bin/llvm-ar-13",
        ),
        tool_path(
            name = "cpp",
            path = "/usr/bin/clang++-13",
        ),
        tool_path(
            name = "gcov",
            path = "/usr/bin/llvm-cov-13",
        ),
        tool_path(
            name = "nm",
            path = "/usr/bin/llvm-nm-13",
        ),
        tool_path(
            name = "objdump",
            path = "/usr/bin/llvm-objdump-13",
        ),
        tool_path(
            name = "strip",
            path = "/usr/bin/llvm-strip-13",
        ),
    ]
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = FEATURES,
        cxx_builtin_include_directories = [
            "/usr/include",
            "/usr/lib/llvm-13/include/c++/v1/",
            "/usr/lib/llvm-13/lib/clang/13.0.1_1/include",
            "/usr/lib/llvm-13/lib/clang/13.0.1_1/share",
        ],
        toolchain_identifier = "linux-llvm-toolchain",
        host_system_name = "local",
        target_system_name = "local",
        target_cpu = "k8",
        target_libc = "unknown",
        compiler = "clang",
        abi_version = "unknown",
        abi_libc_version = "unknown",
        tool_paths = tool_paths,
    )

def darwin_llvm_toolchain_impl(ctx):
    # Default paths on macOS when installing LLVM tools via Homebrew.
    tool_paths = [
        tool_path(
            name = "gcc",
            path = "/usr/local/opt/llvm@13/bin/clang",
        ),
        tool_path(
            name = "ld",
            path = "/usr/local/opt/llvm@13/bin/ld64.lld",
        ),
        tool_path(
            name = "ar",
            path = "/usr/local/opt/llvm@13/bin/llvm-ar",
        ),
        tool_path(
            name = "cpp",
            path = "/usr/local/opt/llvm@13/bin/clang++",
        ),
        tool_path(
            name = "gcov",
            path = "/usr/local/opt/llvm@13/bin/llvm-cov",
        ),
        tool_path(
            name = "nm",
            path = "/usr/local/opt/llvm@13/bin/llvm-nm",
        ),
        tool_path(
            name = "objdump",
            path = "/usr/local/opt/llvm@13/bin/llvm-objdump",
        ),
        tool_path(
            name = "strip",
            path = "/usr/local/opt/llvm@13/bin/llvm-strip",
        ),
    ]
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = FEATURES,
        cxx_builtin_include_directories = [
            "/usr/local/opt/llvm@13/include/c++/v1",
            "/usr/local/Cellar/llvm@13/13.0.1/include/",
            "/usr/local/Cellar/llvm@13/13.0.1/lib/clang/13.0.1/include",
            "/usr/local/Cellar/llvm@13/13.0.1/lib/clang/13.0.1/share",
            "/usr/local/Cellar/llvm@13/13.0.1/lib/clang/13.0.1/include",
            "/Library/Developer/CommandLineTools/SDKs/MacOSX12.sdk/usr/include",
            "/Library/Developer/CommandLineTools/SDKs/MacOSX12.sdk/System/Library/Frameworks",
        ],
        toolchain_identifier = "darwin-llvm-toolchain",
        host_system_name = "local",
        target_system_name = "local",
        target_cpu = "darwin",
        target_libc = "unknown",
        compiler = "clang",
        abi_version = "unknown",
        abi_libc_version = "unknown",
        tool_paths = tool_paths,
    )

cc_toolchain_config_linux = rule(
    implementation = linux_llvm_toolchain_impl,
    attrs = {},
    provides = [CcToolchainConfigInfo],
)

cc_toolchain_config_darwin = rule(
    implementation = darwin_llvm_toolchain_impl,
    attrs = {},
    provides = [CcToolchainConfigInfo],
)

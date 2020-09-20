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

features = [
    feature(
        name = "default_linker_flags",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = [ACTION_NAMES.cpp_link_executable],
                flag_groups = ([
                    flag_group(
                        flags = ["-lstdc++"],
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
                actions = [
                    ACTION_NAMES.cpp_compile,
                ],
                flag_groups = ([
                    flag_group(
                        flags = [
                            "-std=c++20",
                            "-Wall",
                            "-Wextra",
                            "-Wpedantic",
                            "-fno-exceptions",
                            "-fno-rtti",
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
            path = "/usr/bin/clang-10",
        ),
        tool_path(
            name = "ld",
            path = "/usr/bin/lld-10",
        ),
        tool_path(
            name = "ar",
            path = "/usr/bin/llvm-ar-10",
        ),
        tool_path(
            name = "cpp",
            path = "/usr/bin/clang++-10",
        ),
        tool_path(
            name = "gcov",
            path = "/usr/bin/llvm-cov-10",
        ),
        tool_path(
            name = "nm",
            path = "/usr/bin/llvm-nm-10",
        ),
        tool_path(
            name = "objdump",
            path = "/usr/bin/llvm-objdump-10",
        ),
        tool_path(
            name = "strip",
            path = "/usr/bin/llvm-strip-10",
        ),
    ]
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features,
        cxx_builtin_include_directories = [
            "/usr/lib/llvm-10/lib/clang/10.0.1/include",
            "/usr/include",
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
            path = "/usr/local/opt/llvm/bin/clang",
        ),
        tool_path(
            name = "ld",
            path = "/usr/local/opt/llvm/bin/lld",
        ),
        tool_path(
            name = "ar",
            path = "/usr/local/opt/llvm/bin/llvm-ar",
        ),
        tool_path(
            name = "cpp",
            path = "/usr/local/opt/llvm/bin/clang++",
        ),
        tool_path(
            name = "gcov",
            path = "/usr/local/opt/llvm/bin/llvm-cov",
        ),
        tool_path(
            name = "nm",
            path = "/usr/local/opt/llvm/bin/llvm-nm",
        ),
        tool_path(
            name = "objdump",
            path = "/usr/local/opt/llvm/bin/llvm-objdump",
        ),
        tool_path(
            name = "strip",
            path = "/usr/local/opt/llvm/bin/llvm-strip",
        ),
    ]
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features,
        cxx_builtin_include_directories = [
            "/usr/local/opt/llvm/include/c++/v1",
            "/usr/local/Cellar/llvm/10.0.1_1/lib/clang/10.0.1/include",
            "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include",
            "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/System/Library/Frameworks",
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

def windows_llvm_toolchain_impl(ctx):
    # Default paths on Windows for the MSVC LLVM tools.
    tool_paths = [
        tool_path(
            name = "gcc",
            path = "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Tools/Llvm/bin/clang.exe",
        ),
        tool_path(
            name = "ld",
            path = "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Tools/Llvm/bin/lld.exe",
        ),
        tool_path(
            name = "ar",
            path = "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Tools/Llvm/bin/llvm-ar.exe",
        ),
        tool_path(
            name = "cpp",
            path = "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Tools/Llvm/bin/clang++.exe",
        ),
        tool_path(
            name = "gcov",
            path = "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Tools/Llvm/bin/llvm-cov.exe",
        ),
        tool_path(
            name = "nm",
            path = "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Tools/Llvm/bin/llvm-nm.exe",
        ),
        tool_path(
            name = "objdump",
            path = "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Tools/Llvm/bin/llvm-objdump.exe",
        ),
        tool_path(
            name = "strip",
            path = "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Tools/Llvm/bin/llvm-strip.exe",
        ),
    ]
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features,
        cxx_builtin_include_directories = [
            "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Tools/Llvm/lib/clang/10.0.0/include",
            "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Tools/MSVC/14.27.29110/include",
        ],
        toolchain_identifier = "windows-llvm-toolchain",
        host_system_name = "local",
        target_system_name = "local",
        target_cpu = "x64_windows",
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

cc_toolchain_config_windows = rule(
    implementation = windows_llvm_toolchain_impl,
    attrs = {},
    provides = [CcToolchainConfigInfo],
)

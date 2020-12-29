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

UNIVERSAL_LINKER_FLAGS = [
    "-lc++",
    # "-nostdinc",
    # "-nostdinc++",
    "-lpthread",
    "-lm",
    "-lz",
]

UNIVERSAL_CXX_COMPILER_FLAGS = [
    "-std=c++20",
    "-stdlib=libc++",
    "-pthread",
    "-fno-exceptions",
    "-fno-rtti",
    "-nostdinc",
    "-nostdinc++",
    # "-nostdlibinc",
    "-Wall",
    "-Wextra",
    "-pedantic",
    "-Wno-gnu-zero-variadic-macro-arguments",  # gMock
    "-Wno-gcc-compat",  # absl::StrFormat.
]

# @bazel_tools//src/conditions:windows

APPLE_LLVM_HOMEBREW_INCLUDE_DIRECTORIES = [
    "/usr/local/opt/llvm/include/c++/v1",
    "/usr/local/Cellar/llvm/11.1.0/include/",
    "/usr/local/Cellar/llvm/11.1.0/lib/clang/11.1.0/include",
    "/usr/local/Cellar/llvm/11.1.0/lib/clang/11.1.0/share",
]

MACOS_X86_64_INCLUDE_DIRECTORIES = [
    "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include",
    "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/lib",
    "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks",
    # TODO: We shouldn't need this:
    "/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include",
]

IOS_ARM64_INCLUDE_DIRECTORIES = [
    "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/usr/include",
    "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/usr/lib",
    "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/System/Library/Frameworks",
]

IOS_X86_64_INCLUDE_DIRECTORIES = [
    "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr/include",
    "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr/lib",
    "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/System/Library/Frameworks",
]

MACOS_X86_64_FRAMEWORK_DIRECTORIES = [
    "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks",
]

IOS_ARM64_FRAMEWORK_DIRECTORIES = [
    "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/System/Library/Frameworks",
]

IOS_X86_64_FRAMEWORK_DIRECTORIES = [
    "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/System/Library/Frameworks",
]

def prepend(str, list):
  result = []
  for item in list:
    result.append(str)
    result.append(item)
  return result

MACOS_X86_64_CXX_COMPILER_FLAGS = UNIVERSAL_CXX_COMPILER_FLAGS + \
    prepend("-isystem", APPLE_LLVM_HOMEBREW_INCLUDE_DIRECTORIES) + \
    prepend("-isystem", MACOS_X86_64_INCLUDE_DIRECTORIES) + \
    prepend("-iframework", MACOS_X86_64_FRAMEWORK_DIRECTORIES) + [
        "-target", "x86_64-apple-darwin",
    ]

IOS_ARM64_CXX_COMPILER_FLAGS = UNIVERSAL_CXX_COMPILER_FLAGS + \
    prepend("-isystem", APPLE_LLVM_HOMEBREW_INCLUDE_DIRECTORIES) + \
    prepend("-isystem", IOS_ARM64_INCLUDE_DIRECTORIES) + \
    prepend("-iframework", IOS_ARM64_FRAMEWORK_DIRECTORIES) + [
        "-target", "arm64-apple-darwin",
        "-miphoneos-version-min=13.0",
    ]

IOS_X86_64_CXX_COMPILER_FLAGS = UNIVERSAL_CXX_COMPILER_FLAGS + \
    prepend("-isystem", APPLE_LLVM_HOMEBREW_INCLUDE_DIRECTORIES) + \
    prepend("-isystem", IOS_X86_64_INCLUDE_DIRECTORIES) + \
    prepend("-iframework", IOS_X86_64_FRAMEWORK_DIRECTORIES) + [
        "-target", "x86_64-apple-darwin",
        "-miphoneos-version-min=13.0",
    ]

def features(linker_flags, compiler_flags):
  return [
      feature(
          name = "default_linker_flags",
          enabled = True,
          flag_sets = [
              flag_set(
                  actions = [ACTION_NAMES.cpp_link_executable],
                  flag_groups = ([
                      flag_group(
                          flags = linker_flags,
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
                          flags = compiler_flags,
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
            path = "/usr/bin/clang-11",
        ),
        tool_path(
            name = "ld",
            path = "/usr/bin/lld-11",
        ),
        tool_path(
            name = "ar",
            path = "/usr/bin/llvm-ar-11",
        ),
        tool_path(
            name = "cpp",
            path = "/usr/bin/clang++-11",
        ),
        tool_path(
            name = "gcov",
            path = "/usr/bin/llvm-cov-11",
        ),
        tool_path(
            name = "nm",
            path = "/usr/bin/llvm-nm-11",
        ),
        tool_path(
            name = "objdump",
            path = "/usr/bin/llvm-objdump-11",
        ),
        tool_path(
            name = "strip",
            path = "/usr/bin/llvm-strip-11",
        ),
    ]
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features(UNIVERSAL_LINKER_FLAGS, UNIVERSAL_CXX_COMPILER_FLAGS),
        cxx_builtin_include_directories = [
            "/usr/include",
            "/usr/lib/llvm-11/include/c++/v1/",
            "/usr/lib/llvm-11/lib/clang/11.1.0_1/include",
            "/usr/lib/llvm-11/lib/clang/11.1.0_1/share",
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

# Default paths on macOS when installing LLVM tools via Homebrew.
APPLE_HOMEBREW_LLVM_TOOL_PATHS = [
    tool_path(
        name = "gcc",
        path = "/usr/local/opt/llvm/bin/clang",
    ),
    tool_path(
        name = "ld",
        path = "/usr/local/opt/llvm/bin/ld64.lld",
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

def macos_x86_64_llvm_toolchain_impl(ctx):
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features(UNIVERSAL_LINKER_FLAGS, MACOS_X86_64_CXX_COMPILER_FLAGS),
        cxx_builtin_include_directories =
          APPLE_LLVM_HOMEBREW_INCLUDE_DIRECTORIES + MACOS_X86_64_INCLUDE_DIRECTORIES,
        toolchain_identifier = "macos-x86_64-llvm-toolchain",
        host_system_name = "local",
        target_system_name = "local",
        target_cpu = "darwin",
        target_libc = "unknown",
        compiler = "clang",
        abi_version = "unknown",
        abi_libc_version = "unknown",
        tool_paths = APPLE_HOMEBREW_LLVM_TOOL_PATHS,
    )

def ios_arm64_llvm_toolchain_impl(ctx):
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features(UNIVERSAL_LINKER_FLAGS, IOS_ARM64_CXX_COMPILER_FLAGS),
        cxx_builtin_include_directories =
          APPLE_LLVM_HOMEBREW_INCLUDE_DIRECTORIES + IOS_ARM64_INCLUDE_DIRECTORIES,
        toolchain_identifier = "ios-arm64-llvm-toolchain",
        host_system_name = "local",
        target_system_name = "local",
        target_cpu = "ios_arm64",
        target_libc = "unknown",
        compiler = "clang",
        abi_version = "unknown",
        abi_libc_version = "unknown",
        tool_paths = APPLE_HOMEBREW_LLVM_TOOL_PATHS,
    )

def ios_x86_64_llvm_toolchain_impl(ctx):
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features(UNIVERSAL_LINKER_FLAGS, IOS_X86_64_CXX_COMPILER_FLAGS),
        cxx_builtin_include_directories =
          APPLE_LLVM_HOMEBREW_INCLUDE_DIRECTORIES + IOS_X86_64_INCLUDE_DIRECTORIES,
        toolchain_identifier = "ios-x86_64-llvm-toolchain",
        host_system_name = "local",
        target_system_name = "local",
        target_cpu = "ios_x86_64",
        target_libc = "unknown",
        compiler = "clang",
        abi_version = "unknown",
        abi_libc_version = "unknown",
        tool_paths = APPLE_HOMEBREW_LLVM_TOOL_PATHS,
    )

cc_toolchain_config_linux = rule(
    implementation = linux_llvm_toolchain_impl,
    attrs = {},
    provides = [CcToolchainConfigInfo],
)

cc_toolchain_config_macos_x86_64 = rule(
    implementation = macos_x86_64_llvm_toolchain_impl,
    attrs = {},
    provides = [CcToolchainConfigInfo],
)

cc_toolchain_config_ios_arm64 = rule(
    implementation = ios_arm64_llvm_toolchain_impl,
    attrs = {},
    provides = [CcToolchainConfigInfo],
)

cc_toolchain_config_ios_x86_64 = rule(
    implementation = ios_x86_64_llvm_toolchain_impl,
    attrs = {},
    provides = [CcToolchainConfigInfo],
)

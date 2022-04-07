# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

load("@rules_python//python:defs.bzl", "py_library", "py_test")
load("@pip_deps_dev//:requirements.bzl", "requirement")
load(
    "//spoor/runtime/wrappers/apple:config.bzl",
    "RUNTIME_STUB_TARGET_NAME",
    "RUNTIME_STUB_XCFRAMEWORK_FILES",
    "RUNTIME_TARGET_NAME",
    "RUNTIME_XCFRAMEWORK_FILES",
)

TOOLCHAIN_NAME = "MicrosoftSpoor.xctoolchain"

_INSTRUMENT_APP_TAGS = [
    # Both Bazel and `xcodebuild` use `sandbox-exec`, however, `sandbox-exec`
    # does not support nesting.
    "no-sandbox",
]

# All executable wrappers are py_libraries so they emit only one file for the
# genrule location expansion.

filegroup(
    name = "toolchain_info",
    srcs = ["ToolchainInfo.plist"],
    visibility = ["//visibility:private"],
)

py_library(
    name = "init",
    srcs = ["__init__.py"],
    visibility = ["//visibility:private"],
)

py_library(
    name = "shared",
    srcs = ["shared.py"],
    visibility = ["//visibility:private"],
)

py_test(
    name = "shared_test",
    srcs = ["shared_test.py"],
    data = ["//toolchain/xcode/test_data/build_args"],
    visibility = ["//visibility:private"],
    deps = [
        ":shared",
        requirement("pytest"),
    ],
)

py_library(
    name = "clang_clangxx_shared",
    srcs = ["clang_clangxx_shared.py"],
    visibility = ["//visibility:private"],
)

py_library(
    name = "clang",
    srcs = ["clang.py"],
    visibility = ["//visibility:private"],
    deps = [":shared"],
)

py_library(
    name = "clangxx",
    srcs = ["clangxx.py"],
    visibility = ["//visibility:private"],
    deps = [":shared"],
)

py_test(
    name = "clang_clangxx_test",
    srcs = ["clang_clangxx_test.py"],
    data = [
        "//toolchain/xcode/test_data/build_args",
        "//toolchain/xcode/test_data/info_plists",
    ],
    visibility = ["//visibility:private"],
    deps = [
        ":clang",
        ":clang_clangxx_shared",
        ":clangxx",
        ":shared",
        requirement("pytest"),
    ],
)

py_library(
    name = "swift",
    srcs = ["swift.py"],
    visibility = ["//visibility:private"],
    deps = [":shared"],
)

py_test(
    name = "swift_test",
    srcs = ["swift_test.py"],
    data = ["//toolchain/xcode/test_data/build_args"],
    visibility = ["//visibility:private"],
    deps = [
        ":swift",
        requirement("pytest"),
    ],
)

py_library(
    name = "swiftc",
    srcs = ["swiftc.py"],
    visibility = ["//visibility:private"],
    deps = [":shared"],
)

py_test(
    name = "swiftc_test",
    srcs = ["swiftc_test.py"],
    data = ["//toolchain/xcode/test_data/build_args"],
    visibility = ["//visibility:private"],
    deps = [
        ":swiftc",
        requirement("pytest"),
    ],
)

_TOOLCHAIN_TARGETS_TO_OUTS = [
    [
        ":toolchain_info",
        TOOLCHAIN_NAME + "/ToolchainInfo.plist",
    ],
    [
        ":init",
        TOOLCHAIN_NAME + "/usr/bin/__init__.py",
    ],
    [
        ":clang",
        TOOLCHAIN_NAME + "/usr/bin/clang",
    ],
    [
        ":clangxx",
        TOOLCHAIN_NAME + "/usr/bin/clang++",
    ],
    [
        ":swift",
        TOOLCHAIN_NAME + "/usr/bin/swift",
    ],
    [
        ":swiftc",
        TOOLCHAIN_NAME + "/usr/bin/swiftc",
    ],
    [
        ":clang_clangxx_shared",
        TOOLCHAIN_NAME + "/usr/bin/clang_clangxx_shared.py",
    ],
    [
        ":shared",
        TOOLCHAIN_NAME + "/usr/bin/shared.py",
    ],
    [
        "//spoor/instrumentation/wrappers/apple:spoor_opt",
        TOOLCHAIN_NAME + "/spoor/bin/spoor_opt",
    ],
]

_RUNTIME_XCFRAMEWORK_TARGET = "//spoor/runtime/wrappers/apple:{}".format(
    RUNTIME_TARGET_NAME,
)

_RUNTIME_STUB_XCFRAMEWORK_TARGET = "//spoor/runtime/wrappers/apple:{}".format(
    RUNTIME_STUB_TARGET_NAME,
)

_FRAMEWORKS_PATH = "{}/spoor/frameworks".format(TOOLCHAIN_NAME)

_XCFRAMEWORKS_FILES = [
    "{}/{}".format(_FRAMEWORKS_PATH, file)
    for file in RUNTIME_XCFRAMEWORK_FILES
] + [
    "{}/{}".format(_FRAMEWORKS_PATH, file)
    for file in RUNTIME_STUB_XCFRAMEWORK_FILES
]

genrule(
    name = "toolchain",
    srcs = [src for (src, _) in _TOOLCHAIN_TARGETS_TO_OUTS] + [
        _RUNTIME_XCFRAMEWORK_TARGET,
        _RUNTIME_STUB_XCFRAMEWORK_TARGET,
    ],
    # Note: Bazel forces all outputs to be executable.
    # Ticket: https://github.com/bazelbuild/bazel/issues/3359
    # Additional context:
    #   https://github.com/bazelbuild/bazel/issues/5588#issuecomment-628689827
    outs = [out for (_, out) in _TOOLCHAIN_TARGETS_TO_OUTS] +
           _XCFRAMEWORKS_FILES,
    cmd = " && ".join(
        [
            "cp $(location {}) $(@D)/{}".format(src, out)
            for (src, out) in _TOOLCHAIN_TARGETS_TO_OUTS
        ] + [
            "unzip $(location {}) -d $(@D)/{}".format(
                _RUNTIME_XCFRAMEWORK_TARGET,
                _FRAMEWORKS_PATH,
            ),
            "unzip $(location {}) -d $(@D)/{}".format(
                _RUNTIME_STUB_XCFRAMEWORK_TARGET,
                _FRAMEWORKS_PATH,
            ),
        ],
    ),
    visibility = ["//visibility:public"],
)

sh_test(
    name = "instrument_minimal_objc_ios_app_test",
    size = "large",
    srcs = ["instrument_ios_app_test.sh"],
    data = [
        ":toolchain",
        "//toolchain/xcode/test_data/minimal_objc_ios_app",
    ],
    env = {
        "APP_NAME": "MinimalObjcApp",
        "APP_RELATIVE_PATH": "toolchain/xcode/test_data/minimal_objc_ios_app",
        "TOOLCHAIN_NAME": TOOLCHAIN_NAME,
    },
    tags = _INSTRUMENT_APP_TAGS,
    target_compatible_with = ["@platforms//os:macos"],
    visibility = ["//visibility:private"],
)

sh_test(
    name = "instrument_minimal_swift_ios_app_test",
    size = "large",
    srcs = ["instrument_ios_app_test.sh"],
    data = [
        ":toolchain",
        "//toolchain/xcode/test_data/minimal_swift_ios_app",
    ],
    env = {
        "APP_NAME": "MinimalSwiftApp",
        "APP_RELATIVE_PATH": "toolchain/xcode/test_data/minimal_swift_ios_app",
        "TOOLCHAIN_NAME": TOOLCHAIN_NAME,
    },
    tags = _INSTRUMENT_APP_TAGS,
    target_compatible_with = ["@platforms//os:macos"],
    visibility = ["//visibility:private"],
)

sh_test(
    name = "instrument_wikipedia_ios_app_test",
    size = "enormous",
    srcs = ["instrument_ios_app_test.sh"],
    data = [
        ":toolchain",
        "@org_wikimedia_wikipedia_ios//:wikipedia_ios",
    ],
    env = {
        "APP_NAME": "Wikipedia",
        "APP_RELATIVE_PATH": "external/org_wikimedia_wikipedia_ios",
        "TOOLCHAIN_NAME": TOOLCHAIN_NAME,
    },
    tags = _INSTRUMENT_APP_TAGS,
    target_compatible_with = ["@platforms//os:macos"],
    visibility = ["//visibility:private"],
)

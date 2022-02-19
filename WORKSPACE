# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

workspace(name = "spoor")

load(
    "@bazel_tools//tools/build_defs/repo:http.bzl",
    "http_archive",
    "http_file",
)

http_archive(
    name = "com_microsoft_gsl",
    build_file = "//toolchain:gsl.BUILD",
    sha256 = "ca3f015ea80a8d9163714dbf6b377a424e196bd5c8b254fdb5e5770ea3f84a55",
    strip_prefix = "GSL-ec6cd75d57f68b6566e1d406de20e59636a881e7",
    url = "https://github.com/microsoft/GSL/archive/ec6cd75d57f68b6566e1d406de20e59636a881e7.tar.gz",
)

http_archive(
    name = "rules_python",
    sha256 = "a30abdfc7126d497a7698c29c46ea9901c6392d6ed315171a6df5ce433aa4502",
    strip_prefix = "rules_python-0.6.0",
    url = "https://github.com/bazelbuild/rules_python/archive/refs/tags/0.6.0.tar.gz",
)

load("@rules_python//python:pip.bzl", "pip_install")

pip_install(
    name = "pip_deps",
    requirements = "//:requirements.txt",
)

pip_install(
    name = "pip_deps_dev",
    requirements = "//:requirements-dev.txt",
)

# TODO(#133): Use LLVM's Bazel build configuration when it is checked in.
http_archive(
    name = "com_google_llvm_bazel",
    sha256 = "910f4c87511d6add2a61128a95ea0afc123ac3a5d79675d4bbbd7d43577020af",
    strip_prefix = "llvm-bazel-lelandjansen-llvm-12.0.1/llvm-bazel",
    url = "https://github.com/lelandjansen/llvm-bazel/archive/refs/heads/lelandjansen/llvm-12.0.1.tar.gz",
)

http_archive(
    name = "org_llvm_llvm_project",
    build_file_content = "#empty",
    patch_cmds = [
        """
        if [[ "$OSTYPE" == "darwin"* ]]; then
          sed -i '' 's|#include "llvm/Transforms/HelloNew/HelloWorld.h"||g' llvm/lib/Passes/PassBuilder.cpp
          sed -i '' 's|FUNCTION_PASS("helloworld", HelloWorldPass())||g' llvm/lib/Passes/PassRegistry.def
        else
          sed -i 's|#include "llvm/Transforms/HelloNew/HelloWorld.h"||g' llvm/lib/Passes/PassBuilder.cpp
          sed -i 's|FUNCTION_PASS("helloworld", HelloWorldPass())||g' llvm/lib/Passes/PassRegistry.def
        fi
        """,
    ],
    sha256 = "66b64aa301244975a4aea489f402f205cde2f53dd722dad9e7b77a0459b4c8df",
    strip_prefix = "llvm-project-llvmorg-12.0.1",
    url = "https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-12.0.1.tar.gz",
)

load("@com_google_llvm_bazel//:terminfo.bzl", "llvm_terminfo_disable")

llvm_terminfo_disable(
    name = "llvm_terminfo",
)

load("@com_google_llvm_bazel//:zlib.bzl", "llvm_zlib_disable")

llvm_zlib_disable(
    name = "llvm_zlib",
)

load("@com_google_llvm_bazel//:configure.bzl", "llvm_configure")

llvm_configure(
    # The LLVM Bazel project's configuration requires deviating from Spoor's
    # project naming convention.
    name = "llvm-project",
    src_path = ".",
    src_workspace = "@org_llvm_llvm_project//:WORKSPACE",
)

http_archive(
    name = "com_apple_swift",
    build_file = "@//toolchain:swift.BUILD",
    patch_cmds = [
        "cat /dev/null > include/swift/Runtime/Config.h",
        """
        if [[ "$OSTYPE" == "darwin"* ]]; then
          sed -i '' 's/SWIFT_RUNTIME_EXPORT//g' include/swift/Demangling/Demangle.h
        else
          sed -i 's/SWIFT_RUNTIME_EXPORT//g' include/swift/Demangling/Demangle.h
        fi
        """,
    ],
    sha256 = "0046ecab640475441251b1cceb3dd167a4c7729852104d7675bdbd75fced6b82",
    strip_prefix = "swift-swift-5.5.2-RELEASE",
    url = "https://github.com/apple/swift/archive/swift-5.5.2-RELEASE.tar.gz",
)

http_archive(
    name = "com_google_absl",
    sha256 = "dcf71b9cba8dc0ca9940c4b316a0c796be8fab42b070bb6b7cab62b48f0e66c4",
    strip_prefix = "abseil-cpp-20211102.0",
    url = "https://github.com/abseil/abseil-cpp/archive/refs/tags/20211102.0.tar.gz",
)

http_archive(
    name = "com_google_protobuf",
    sha256 = "390191a0d7884b3e52bb812c440ad1497b9d484241f37bb8e2ccc8c2b72d6c36",
    strip_prefix = "protobuf-3.19.3",
    url = "https://github.com/protocolbuffers/protobuf/archive/refs/tags/v3.19.3.tar.gz",
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

http_archive(
    name = "com_google_snappy",
    build_file = "//toolchain:snappy.BUILD",
    sha256 = "75c1fbb3d618dd3a0483bff0e26d0a92b495bbe5059c8b4f1c962b478b6e06e7",
    strip_prefix = "snappy-1.1.9",
    url = "https://github.com/google/snappy/archive/1.1.9.tar.gz",
)

http_archive(
    name = "com_google_cityhash",
    build_file = "//toolchain:city_hash.BUILD",
    sha256 = "f70368facd15735dffc77fe2b27ab505bfdd05be5e9166d94149a8744c212f49",
    strip_prefix = "cityhash-8af9b8c2b889d80c22d6bc26ba0df1afb79a30db",
    url = "https://github.com/google/cityhash/archive/8af9b8c2b889d80c22d6bc26ba0df1afb79a30db.tar.gz",
)

http_archive(
    name = "com_marzer_tomlplusplus",
    build_file = "//toolchain:tomlplusplus.BUILD",
    patch_cmds = [
        """
        if [[ "$OSTYPE" == "darwin"* ]]; then
          sed -i '' 's|#pragma clang diagnostic ignored "-Wreserved-identifier"||g' toml.hpp
        else
          sed -i 's|#pragma clang diagnostic ignored "-Wreserved-identifier"||g' toml.hpp
        fi
        """,
    ],
    sha256 = "e05b2814b891e223d7546aa2408d6cba0628164a84ac453205c7743cb667b9cf",
    strip_prefix = "tomlplusplus-3.0.1",
    url = "https://github.com/marzer/tomlplusplus/archive/refs/tags/v3.0.1.tar.gz",
)

http_archive(
    # Perfetto's build config requires deviating from the naming convention.
    name = "perfetto",
    sha256 = "9d2955736ce9d234e0f5153acfefea8facfa762c9167024902ea98f9010207aa",
    strip_prefix = "perfetto-23.0",
    url = "https://github.com/google/perfetto/archive/v23.0.tar.gz",
)

new_local_repository(
    # Perfetto's build config requires deviating from the naming convention.
    name = "perfetto_cfg",
    build_file_content = "",
    path = "toolchain/perfetto_overrides",
)

http_archive(
    name = "com_google_googletest",
    sha256 = "9dc9157a9a1551ec7a7e43daea9a694a0bb5fb8bec81235d8a1e6ef64c716dcb",
    strip_prefix = "googletest-release-1.10.0",
    url = "https://github.com/google/googletest/archive/release-1.10.0.tar.gz",
)

http_archive(
    name = "com_google_benchmark",
    sha256 = "6132883bc8c9b0df5375b16ab520fac1a85dc9e4cf5be59480448ece74b278d4",
    strip_prefix = "benchmark-1.6.1",
    url = "https://github.com/google/benchmark/archive/v1.6.1.tar.gz",
)

http_archive(
    name = "com_bazelbuild_bazel",
    sha256 = "720c42ca793d6ff3050121140c17e3511f7e8306b252ebfb4310b124dbdac10c",
    strip_prefix = "bazel-5.0.0",
    url = "https://github.com/bazelbuild/bazel/archive/5.0.0.tar.gz",
)

http_archive(
    name = "io_bazel_rules_go",
    sha256 = "7a89df64b765721be9bb73b3aa52c15209af3b6628cae4344b9516e8b21c2b8b",
    strip_prefix = "rules_go-0.29.0",
    url = "https://github.com/bazelbuild/rules_go/archive/refs/tags/v0.29.0.tar.gz",
)

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains(version = "1.17.1")

http_archive(
    name = "bazel_gazelle",
    sha256 = "fc4c319b9e32ea44be8a5e1a46746d93e8b6a8b104baf7cb6a344a0a08386fed",
    strip_prefix = "bazel-gazelle-0.24.0",
    url = "https://github.com/bazelbuild/bazel-gazelle/archive/refs/tags/v0.24.0.tar.gz",
)

load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies")

gazelle_dependencies()

http_archive(
    name = "build_bazel_rules_apple",
    sha256 = "c4ebd7ef433967eff4bcff5d6ad30691f9c62b2d3a708938e74a885c950c53b8",
    strip_prefix = "rules_apple-70f528160238828d865d19b45375fedbb1a9ca32",
    url = "https://github.com/bazelbuild/rules_apple/archive/70f528160238828d865d19b45375fedbb1a9ca32.tar.gz",
)

load(
    "@build_bazel_rules_apple//apple:repositories.bzl",
    "apple_rules_dependencies",
)

apple_rules_dependencies()

http_archive(
    name = "org_wikimedia_wikipedia_ios",
    build_file = "//toolchain:wikipedia_ios.BUILD",
    patch_cmds = [
        # Download the project's dependencies as a patch to leverage Bazel's
        # `http_archive` caching.
        """
        xcodebuild clean build \
          -resolvePackageDependencies \
          -clonedSourcePackagesDirPath . \
          -project Wikipedia.xcodeproj \
          -scheme Wikipedia
        """,
        # Hack: Temporarily rename path names with spaces because they are not
        # supported by Bazel's `filegroup`.
        """
        find . -name '* *' -print0 |
          sort -rz |
            while read -d $'\\0' f; do
              mv \"$f\" \"$(dirname \"$f\")/$(basename \"${f// /__SPACE__}\")\"
            done
        """,
    ],
    sha256 = "e72ec3d81559aae7632a154ef5b08668c92fa326c049c83ed9e1e377932d4dbd",
    strip_prefix = "wikipedia-ios-releases-6.8.2",
    url = "https://github.com/wikimedia/wikipedia-ios/archive/refs/tags/releases/6.8.2.tar.gz",
)

# TODO(#179): Use Perfetto's `trace_processor` when the configurable
# installation path is checked in.
http_file(
    name = "dev_perfetto_trace_processor",
    downloaded_file_path = "trace_processor",
    executable = True,
    sha256 = "50d15de361107ec275c9ea994541917171dff10319418de64799628db09083a2",
    urls = ["https://raw.githubusercontent.com/lelandjansen/perfetto/lelandjansen/trace-processor-env/tools/trace_processor"],
)

http_file(
    name = "com_google_style_guide_pylintrc",
    downloaded_file_path = "pylintrc",
    sha256 = "c93e541953066272d590b43d1a86ceeb3c0ef06ed5a7d74b799185ff6162ea9e",
    urls = ["https://raw.githubusercontent.com/google/styleguide/9c8784ded344f6a35d1e1550385002f613a0c788/pylintrc"],
)

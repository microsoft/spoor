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

# LLVM's build config requires deviating from the naming convention.
http_archive(
    name = "bazel_skylib",
    sha256 = "f7be3474d42aae265405a592bb7da8e171919d74c16f082a5457840f06054728",
    url = "https://github.com/bazelbuild/bazel-skylib/releases/download/1.2.1/bazel-skylib-1.2.1.tar.gz",
)

_LLVM_VERSION = "14.0.6"

_LLVM_SHA256 = "98f15f842700bdb7220a166c8d2739a03a72e775b67031205078f39dd756a055"

http_archive(
    name = "org_llvm_llvm_project_raw",
    build_file_content = "# empty",
    sha256 = _LLVM_SHA256,
    strip_prefix = "llvm-project-llvmorg-{version}".format(version = _LLVM_VERSION),
    url = "https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-{version}.tar.gz".format(version = _LLVM_VERSION),
)

load(
    "@org_llvm_llvm_project_raw//utils/bazel:configure.bzl",
    "llvm_configure",
    "llvm_disable_optional_support_deps",
)

llvm_configure(name = "llvm-project")

llvm_disable_optional_support_deps()

http_archive(
    name = "rules_foreign_cc",
    sha256 = "b8db5586c275336930491dba0eead4b3e53b644b1a8e16044aa88de04de27d91",
    strip_prefix = "rules_foreign_cc-0.8.0",
    url = "https://github.com/bazelbuild/rules_foreign_cc/archive/refs/tags/0.8.0.zip",
)

load(
    "@rules_foreign_cc//foreign_cc:repositories.bzl",
    "rules_foreign_cc_dependencies",
)

rules_foreign_cc_dependencies()

http_archive(
    name = "com_apple_swift",
    build_file = "@//toolchain:swift.BUILD",
    patch_cmds = [
        "cat /dev/null > include/swift/Runtime/Config.h",
        """
        if [[ "$OSTYPE" == "darwin"* ]]; then
          sed -i '' 's|#include "../../../stdlib/public/|#include "stdlib/public/|g' include/swift/Demangling/Errors.h
        else
          sed -i 's|#include "../../../stdlib/public/|#include "stdlib/public/|g' include/swift/Demangling/Errors.h
        fi
        """,
    ],
    sha256 = "5e333bfa9db84080d496ea809ae43b8de3868da3e312285265b6a9aa888dc2b0",
    strip_prefix = "swift-4eae6538a9ac141dfc691893d874f3b70e8bcd5a",
    # `release/5.7` branch on 2022-07-06.
    url = "https://github.com/apple/swift/archive/4eae6538a9ac141dfc691893d874f3b70e8bcd5a.tar.gz",
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
    sha256 = "b4870bf121ff7795ba20d20bcdd8627b8e088f2d1dab299a031c1034eddc93d5",
    strip_prefix = "googletest-release-1.11.0",
    url = "https://github.com/google/googletest/archive/release-1.11.0.tar.gz",
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
    sha256 = "90bbacabae2720f4c3b5edd4086f925ddf8630c299b2470f683014a8c978a834",
    strip_prefix = "rules_apple-0.34.0",
    url = "https://github.com/bazelbuild/rules_apple/archive/0.34.0.tar.gz",
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
    sha256 = "74e3ca4f45e5df3d7bfd0729c0e6635078a13a46866fcd652b31be45ea80219d",
    urls = ["https://raw.githubusercontent.com/google/styleguide/25bd3525fa8277d40a0163df04f6a06277e6265b/pylintrc"],
)

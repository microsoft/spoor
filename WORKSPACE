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
    sha256 = "954aa89b491be4a083304a2cb838019c8b8c3720a7abb9c4cb81ac7a24230cea",
    url = "https://github.com/bazelbuild/rules_python/releases/download/0.4.0/rules_python-0.4.0.tar.gz",
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
    sha256 = "2eb64e813477e8009f4813aae5ae7f95090585a9a06f2437007e387551769dea",
    strip_prefix = "llvm-bazel-lelandjansen-llvm-11.1.0/llvm-bazel",
    url = "https://github.com/lelandjansen/llvm-bazel/archive/refs/heads/lelandjansen/llvm-11.1.0.tar.gz",
)

http_archive(
    name = "org_llvm_llvm_project",
    build_file_content = "#empty",
    sha256 = "d4dbca22c0056847a89d4335c172ecc14db8ef7c60f3a639b3cb91cb82961900",
    strip_prefix = "llvm-project-1fdec59bffc11ae37eb51a1b9869f0696bfd5312",
    url = "https://github.com/llvm/llvm-project/archive/1fdec59bffc11ae37eb51a1b9869f0696bfd5312.tar.gz",
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
        "cat /dev/null > include/swift/Basic/STLExtras.h",
        "cat /dev/null > include/swift/Runtime/Config.h",
        "cat /dev/null > include/swift/Runtime/CMakeConfig.h",
        """
        if [[ "$OSTYPE" == "darwin"* ]]; then
          sed -i '' 's/SWIFT_RUNTIME_EXPORT//g' include/swift/Demangling/Demangle.h
        else
          sed -i 's/SWIFT_RUNTIME_EXPORT//g' include/swift/Demangling/Demangle.h
        fi
        """,
    ],
    sha256 = "421dafdb0dd4c55cdfed4d8736e965b42a0d97f690bb13528947f9cc3f7ddca9",
    strip_prefix = "swift-swift-5.4-RELEASE",
    url = "https://github.com/apple/swift/archive/swift-5.4-RELEASE.tar.gz",
)

http_archive(
    name = "com_google_absl",
    sha256 = "59b862f50e710277f8ede96f083a5bb8d7c9595376146838b9580be90374ee1f",
    strip_prefix = "abseil-cpp-20210324.2",
    url = "https://github.com/abseil/abseil-cpp/archive/refs/tags/20210324.2.tar.gz",
)

http_archive(
    name = "com_google_protobuf",
    sha256 = "14e8042b5da37652c92ef6a2759e7d2979d295f60afd7767825e3de68c856c54",
    strip_prefix = "protobuf-3.18.0",
    url = "https://github.com/protocolbuffers/protobuf/archive/refs/tags/v3.18.0.tar.gz",
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
    sha256 = "2e246ee126cfb7bd68edd7285d5bb5c8c5296121ce809306ee71cfd6127c76a6",
    strip_prefix = "tomlplusplus-2.5.0",
    url = "https://github.com/marzer/tomlplusplus/archive/refs/tags/v2.5.0.tar.gz",
)

http_archive(
    # Perfetto's build config requires deviating from the naming convention.
    name = "perfetto",
    sha256 = "9f7f64733eac7021c71742635dba8db888f668af236f62dcf76742318b682c47",
    strip_prefix = "perfetto-15.0",
    urls = ["https://github.com/google/perfetto/archive/v15.0.tar.gz"],
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
    sha256 = "1f71c72ce08d2c1310011ea6436b31e39ccab8c2db94186d26657d41747c85d6",
    strip_prefix = "benchmark-1.6.0",
    url = "https://github.com/google/benchmark/archive/v1.6.0.tar.gz",
)

http_archive(
    name = "com_bazelbuild_bazel",
    sha256 = "7218ae58d0225582d38cc2fbeb6d48f9532e6cff7f4288828e055dae4324ab5b",
    strip_prefix = "bazel-4.2.1",
    url = "https://github.com/bazelbuild/bazel/archive/4.2.1.tar.gz",
)

http_archive(
    # Dependencies require deviating from Spoor's project naming convention.
    name = "io_bazel_rules_go",
    sha256 = "52d0a57ea12139d727883c2fef03597970b89f2cc2a05722c42d1d7d41ec065b",
    url = "https://github.com/bazelbuild/rules_go/releases/download/v0.24.13/rules_go-v0.24.13.tar.gz",
)

http_archive(
    # Dependencies require deviating from Spoor's project naming convention.
    name = "bazel_gazelle",
    sha256 = "222e49f034ca7a1d1231422cdb67066b885819885c356673cb1f72f748a3c9d4",
    url = "https://github.com/bazelbuild/bazel-gazelle/releases/download/v0.22.3/bazel-gazelle-v0.22.3.tar.gz",
)

load(
    "@io_bazel_rules_go//go:deps.bzl",
    "go_register_toolchains",
    "go_rules_dependencies",
)

go_rules_dependencies()

go_register_toolchains()

load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies")

gazelle_dependencies()

http_archive(
    name = "build_bazel_rules_apple",
    sha256 = "d82e8687f473139ccdd6adbf94ab167af39d550dcefabcc79a7085cbf804dbba",
    strip_prefix = "rules_apple-4a0caf9abd43380b86022e9d4e92194fba0b04fe",
    url = "https://github.com/bazelbuild/rules_apple/archive/4a0caf9abd43380b86022e9d4e92194fba0b04fe.tar.gz",
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
    sha256 = "dec60361d82b7d551ffe0f3e3d8d381047f91550c635accf2ec7b858483358ba",
    strip_prefix = "wikipedia-ios-releases-6.8.1",
    url = "https://github.com/wikimedia/wikipedia-ios/archive/refs/tags/releases/6.8.1.tar.gz",
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

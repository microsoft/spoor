# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

workspace(name = "spoor")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_microsoft_gsl",
    build_file = "//toolchain:gsl.BUILD",
    sha256 = "ca3f015ea80a8d9163714dbf6b377a424e196bd5c8b254fdb5e5770ea3f84a55",
    strip_prefix = "GSL-ec6cd75d57f68b6566e1d406de20e59636a881e7",
    url = "https://github.com/microsoft/GSL/archive/ec6cd75d57f68b6566e1d406de20e59636a881e7.tar.gz",
)

http_archive(
    name = "rules_python",
    sha256 = "778197e26c5fbeb07ac2a2c5ae405b30f6cb7ad1f5510ea6fdac03bded96cc6f",
    url = "https://github.com/bazelbuild/rules_python/releases/download/0.2.0/rules_python-0.2.0.tar.gz",
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

# TODO(#132): Upgrade Abseil when the linker issue is resolved in a later
# release.
http_archive(
    name = "com_google_absl",
    sha256 = "ebe2ad1480d27383e4bf4211e2ca2ef312d5e6a09eba869fd2e8a5c5d553ded2",
    strip_prefix = "abseil-cpp-20200923.3",
    url = "https://github.com/abseil/abseil-cpp/archive/20200923.3.tar.gz",
)

http_archive(
    name = "com_google_protobuf",
    sha256 = "9b57647b898e45253c98fae35146f6a5e9e788817d29019f9280270c951a0038",
    strip_prefix = "protobuf-3.15.8",
    url = "https://github.com/protocolbuffers/protobuf/releases/download/v3.15.8/protobuf-cpp-3.15.8.tar.gz",
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

http_archive(
    name = "com_google_snappy",
    build_file = "//toolchain:snappy.BUILD",
    sha256 = "16b677f07832a612b0836178db7f374e414f94657c138e6993cbfc5dcc58651f",
    strip_prefix = "snappy-1.1.8",
    url = "https://github.com/google/snappy/archive/1.1.8.tar.gz",
)

http_archive(
    name = "com_google_cityhash",
    build_file = "//toolchain:city_hash.BUILD",
    sha256 = "f70368facd15735dffc77fe2b27ab505bfdd05be5e9166d94149a8744c212f49",
    strip_prefix = "cityhash-8af9b8c2b889d80c22d6bc26ba0df1afb79a30db",
    url = "https://github.com/google/cityhash/archive/8af9b8c2b889d80c22d6bc26ba0df1afb79a30db.tar.gz",
)

http_archive(
    name = "com_google_googletest",
    sha256 = "9dc9157a9a1551ec7a7e43daea9a694a0bb5fb8bec81235d8a1e6ef64c716dcb",
    strip_prefix = "googletest-release-1.10.0",
    url = "https://github.com/google/googletest/archive/release-1.10.0.tar.gz",
)

http_archive(
    name = "com_google_benchmark",
    sha256 = "dccbdab796baa1043f04982147e67bb6e118fe610da2c65f88912d73987e700c",
    strip_prefix = "benchmark-1.5.2",
    url = "https://github.com/google/benchmark/archive/v1.5.2.tar.gz",
)

http_archive(
    name = "com_bazelbuild_bazel",
    sha256 = "2b9999d06466815ab1f2eb9c6fc6fceb6061efc715b4086fa99eac041976fb4f",
    strip_prefix = "bazel-4.0.0",
    url = "https://github.com/bazelbuild/bazel/archive/4.0.0.tar.gz",
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
    sha256 = "9e81657e9d8b9a96c726e0ce26581f2f141197b504fffdbc0a2d755b688ec386",
    strip_prefix = "rules_apple-0.31.1",
    url = "https://github.com/bazelbuild/rules_apple/archive/0.31.1.tar.gz",
)

load(
    "@build_bazel_rules_apple//apple:repositories.bzl",
    "apple_rules_dependencies",
)

apple_rules_dependencies()

# TODO(#128): Remove this custom `xctestrunner` in favor of the default runner
# when the Python 3.9 support is checked in.
http_archive(
    name = "xctestrunner",
    sha256 = "adcf171ef05af1e7d75cf89373fad9b40f23068531087ddce614ad82b0385f5d",
    strip_prefix = "xctestrunner-fix-compatibility-with-python-3.9",
    url = "https://github.com/thii/xctestrunner/archive/refs/heads/fix-compatibility-with-python-3.9.tar.gz",
)

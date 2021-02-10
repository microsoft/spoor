# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

workspace(name = "spoor")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_microsoft_gsl",
    build_file = "//toolchain:gsl.BUILD",
    sha256 = "ca3f015ea80a8d9163714dbf6b377a424e196bd5c8b254fdb5e5770ea3f84a55",
    strip_prefix = "GSL-ec6cd75d57f68b6566e1d406de20e59636a881e7",
    urls = ["https://github.com/microsoft/GSL/archive/ec6cd75d57f68b6566e1d406de20e59636a881e7.tar.gz"],
)

# TODO(#54): Migrate to the original repository when the fork's changes with
# LLVM 11 support are checked in.
http_archive(
    name = "llvm",
    sha256 = "bd878e859227850e7e529d02bfa6ebf247c463671815ae73912fed8c373b8b44",
    strip_prefix = "bazel_llvm-eb12c9841cae08461e0f1ca03fc43cd9e788064b",
    urls = ["https://github.com/lelandjansen/bazel_llvm/archive/eb12c9841cae08461e0f1ca03fc43cd9e788064b.tar.gz"],
)

load("@llvm//tools/bzl:deps.bzl", "llvm_deps")

llvm_deps()

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
    sha256 = "f32b9dd541fbf3a412123138eb8aaf0fa793d866779c6c3cd5df6621788258c3",
    strip_prefix = "swift-swift-5.3.3-RELEASE",
    urls = ["https://github.com/apple/swift/archive/swift-5.3.3-RELEASE.tar.gz"],
)

http_archive(
    name = "com_google_absl",
    sha256 = "bf3f13b13a0095d926b25640e060f7e13881bd8a792705dd9e161f3c2b9aa976",
    strip_prefix = "abseil-cpp-20200923.2",
    urls = ["https://github.com/abseil/abseil-cpp/archive/20200923.2.tar.gz"],
)

http_archive(
    name = "com_google_protobuf",
    sha256 = "50ec5a07c0c55d4ec536dd49021f2e194a26bfdbc531d03d1e9d4d3e27175659",
    strip_prefix = "protobuf-3.14.0",
    urls = ["https://github.com/protocolbuffers/protobuf/releases/download/v3.14.0/protobuf-cpp-3.14.0.tar.gz"],
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

http_archive(
    name = "com_google_googletest",
    sha256 = "9dc9157a9a1551ec7a7e43daea9a694a0bb5fb8bec81235d8a1e6ef64c716dcb",
    strip_prefix = "googletest-release-1.10.0",
    urls = ["https://github.com/google/googletest/archive/release-1.10.0.tar.gz"],
)

http_archive(
    name = "com_google_benchmark",
    sha256 = "dccbdab796baa1043f04982147e67bb6e118fe610da2c65f88912d73987e700c",
    strip_prefix = "benchmark-1.5.2",
    urls = ["https://github.com/google/benchmark/archive/v1.5.2.tar.gz"],
)

http_archive(
    name = "com_bazelbuild_bazel",
    sha256 = "8d3bf2767f8797efc4ff59a1ad2e3c7dd789a288689bdbc44963d4f025286c98",
    strip_prefix = "bazel-3.7.1",
    urls = ["https://github.com/bazelbuild/bazel/archive/3.7.1.tar.gz"],
)

http_archive(
    # Dependencies require deviating from the naming convention.
    name = "io_bazel_rules_go",
    sha256 = "81eff5df9077783b18e93d0c7ff990d8ad7a3b8b3ca5b785e1c483aacdb342d7",
    urls = ["https://github.com/bazelbuild/rules_go/releases/download/v0.24.9/rules_go-v0.24.9.tar.gz"],
)

http_archive(
    # Dependencies require deviating from the naming convention.
    name = "bazel_gazelle",
    sha256 = "b85f48fa105c4403326e9525ad2b2cc437babaa6e15a3fc0b1dbab0ab064bc7c",
    urls = ["https://github.com/bazelbuild/bazel-gazelle/releases/download/v0.22.2/bazel-gazelle-v0.22.2.tar.gz"],
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

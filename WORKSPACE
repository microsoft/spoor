# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

workspace(name = "spoor")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "rules_foreign_cc",
    sha256 = "2215d55b9a818af7281cba054a897c0f28f34b7fba3365964fa28b10f64963c3",
    strip_prefix = "rules_foreign_cc-eae19778d5fe8605f0f37332a712f05d4a17dc3b",
    urls = ["https://github.com/bazelbuild/rules_foreign_cc/archive/eae19778d5fe8605f0f37332a712f05d4a17dc3b.tar.gz"],
)

load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")

rules_foreign_cc_dependencies()

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
    sha256 = "ebe2ad1480d27383e4bf4211e2ca2ef312d5e6a09eba869fd2e8a5c5d553ded2",
    strip_prefix = "abseil-cpp-20200923.3",
    urls = ["https://github.com/abseil/abseil-cpp/archive/20200923.3.tar.gz"],
)

http_archive(
    name = "com_google_protobuf",
    sha256 = "3ddaffddd73d86e4005cd304c4a4fd962c1819f4c6aa9aad1c3bb2c7f1a35647",
    strip_prefix = "protobuf-3.15.0",
    urls = ["https://github.com/protocolbuffers/protobuf/releases/download/v3.15.0/protobuf-cpp-3.15.0.tar.gz"],
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

_ALL_CONTENT = """\
filegroup(
    name = "all_srcs",
    srcs = glob(["**"]),
    visibility = ["//visibility:public"],
)
"""

http_archive(
    name = "com_google_snappy",
    build_file_content = _ALL_CONTENT,
    sha256 = "16b677f07832a612b0836178db7f374e414f94657c138e6993cbfc5dcc58651f",
    strip_prefix = "snappy-1.1.8",
    urls = ["https://github.com/google/snappy/archive/1.1.8.tar.gz"],
)

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
    sha256 = "2b9999d06466815ab1f2eb9c6fc6fceb6061efc715b4086fa99eac041976fb4f",
    strip_prefix = "bazel-4.0.0",
    urls = ["https://github.com/bazelbuild/bazel/archive/4.0.0.tar.gz"],
)

http_archive(
    # Dependencies require deviating from the naming convention.
    name = "io_bazel_rules_go",
    sha256 = "52d0a57ea12139d727883c2fef03597970b89f2cc2a05722c42d1d7d41ec065b",
    urls = ["https://github.com/bazelbuild/rules_go/releases/download/v0.24.13/rules_go-v0.24.13.tar.gz"],
)

http_archive(
    # Dependencies require deviating from the naming convention.
    name = "bazel_gazelle",
    sha256 = "222e49f034ca7a1d1231422cdb67066b885819885c356673cb1f72f748a3c9d4",
    urls = ["https://github.com/bazelbuild/bazel-gazelle/releases/download/v0.22.3/bazel-gazelle-v0.22.3.tar.gz"],
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

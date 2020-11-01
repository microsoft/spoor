workspace(name = "spoor")

load(
    "@bazel_tools//tools/build_defs/repo:git.bzl",
    "git_repository",
    "new_git_repository",
)

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

new_git_repository(
    name = "com_microsoft_gsl",
    build_file = "@//toolchain:gsl.BUILD",
    commit = "5de956aaf0c80f58b1326591cd59bad72be79556",  # 2020-09-24 (>v3.1.0)
    remote = "https://github.com/microsoft/gsl.git",
    shallow_since = "1600896232 -0700",
)

# http_archive(
#     name = "com_samolisov_bazel_llvm_bridge",
#     sha256 = "d1a471806651511ccde5e24573bcd2433a866f14b86c55ee74dd1b02418641d4",
#     strip_prefix = "bazel-llvm-bridge-release-10-05",
#     url = "https://github.com/samolisov/bazel-llvm-bridge/archive/release/10-05.zip",
# )
# 
# load("@com_samolisov_bazel_llvm_bridge//llvm:llvm_configure.bzl", "llvm_configure")
# 
# llvm_configure(
#     name = "org_llvm_llvm",
#     add_headers_to_deps = True,
# )

# http_archive(
#     name = "com_grail_bazel_toolchain",
#     strip_prefix = "bazel-toolchain-master",
#     urls = ["https://github.com/grailbio/bazel-toolchain/archive/master.tar.gz"],
#     sha256 = "2d305a14280591a10e65bd8af16027f788b7b8491d658674a683f848063b7bc5"
# )
# 
# load("@com_grail_bazel_toolchain//toolchain:deps.bzl", "bazel_toolchain_dependencies")
# 
# bazel_toolchain_dependencies()
# 
# load("@com_grail_bazel_toolchain//toolchain:rules.bzl", "llvm_toolchain")
# 
# llvm_toolchain(
#     name = "llvm_toolchain",
#     llvm_version = "11.0.0",
# )
# 
# load("@llvm_toolchain//:toolchains.bzl", "llvm_register_toolchains")
# 
# llvm_register_toolchains()

load("//toolchain:llvm.bzl", "llvm")
llvm(
    name = "llvm",
    workspace_name = "spoor",
)


# http_archive(
#     name = "com_apple_swift",
#     build_file = "@//toolchain:swift.BUILD",
#     patch_cmds = [
#         "cat /dev/null > include/swift/Runtime/Config.h",
#         "cat /dev/null > include/swift/Runtime/CMakeConfig.h",
#         "sed -i '' 's/SWIFT_RUNTIME_EXPORT//g' include/swift/Demangling/Demangle.h",
#     ],
#     sha256 = "f9e5bd81441c4ec13dd9ea290e2d7b8fe9b30ef66ad68947481022ea5179f83a",
#     strip_prefix = "swift-swift-5.3-RELEASE",
#     urls = ["https://github.com/apple/swift/archive/swift-5.3-RELEASE.tar.gz"],
# )

git_repository(
    name = "com_google_absl",
    commit = "c51510d1d87ebce8615ae1752fd5aca912f6cf4c",  # v20200225.2
    remote = "https://github.com/abseil/abseil-cpp.git",
    shallow_since = "1587584588 -0400",
)

git_repository(
    name = "com_google_protobuf",
    commit = "fde7cf7358ec7cd69e8db9be4f1fa6a5c431386a",  # v3.13.0
    remote = "https://github.com/protocolbuffers/protobuf.git",
    shallow_since = "1597443653 -0700",
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

git_repository(
    name = "com_google_googletest",
    commit = "703bd9caab50b139428cea1aaff9974ebee5742e",  # v1.10.0
    remote = "https://github.com/google/googletest.git",
    shallow_since = "1570114335 -0400",
)

git_repository(
    name = "com_google_benchmark",
    commit = "73d4d5e8d6d449fc8663765a42aa8aeeee844489",  # v1.5.2
    remote = "https://github.com/google/benchmark.git",
    shallow_since = "1599818118 +0100",
)

git_repository(
    name = "com_bazelbuild_bazel",
    commit = "7cfe416c5b702967b63cb2d9e2a2a2aefb8d2cac",  # v3.4.1
    remote = "https://github.com/bazelbuild/bazel.git",
    shallow_since = "1594707848 +0200",
)

git_repository(
    # Dependencies require deviating from the naming convention.
    name = "io_bazel_rules_go",
    commit = "13c17d4aa8c8407049f5644e13c9ace1275d200e",  # v0.23.8
    remote = "https://github.com/bazelbuild/rules_go.git",
    shallow_since = "1597238923 -0400",
)

git_repository(
    # Dependencies require deviating from the naming convention.
    name = "bazel_gazelle",
    commit = "c00612418c4dbc9f3cd35fe71fe1147748048b69",  # v0.21.1
    remote = "https://github.com/bazelbuild/bazel-gazelle",
    shallow_since = "1590680880 -0400",
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

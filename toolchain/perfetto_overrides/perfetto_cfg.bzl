# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Documentation:
# https://github.com/google/perfetto/tree/master/bazel/standalone

PERFETTO_CONFIG = struct(
    root = "//",
    deps = struct(
        build_config = ["//:build_config_hdr"],
        version_header = ["//:cc_perfetto_version_header"],
        zlib = ["@perfetto_dep_zlib//:zlib"],
        jsoncpp = ["@perfetto_dep_jsoncpp//:jsoncpp"],
        linenoise = ["@perfetto_dep_linenoise//:linenoise"],
        sqlite = ["@perfetto_dep_sqlite//:sqlite"],
        sqlite_ext_percentile = ["@perfetto_dep_sqlite_src//:percentile_ext"],
        protoc = ["@com_google_protobuf//:protoc"],
        protoc_lib = ["@com_google_protobuf//:protoc_lib"],
        protobuf_lite = ["@com_google_protobuf//:protobuf_lite"],
        protobuf_full = ["@com_google_protobuf//:protobuf"],
        protobuf_descriptor_proto = ["@com_google_protobuf//:descriptor_proto"]
    ),
    public_visibility = ["//visibility:public"],
    proto_library_visibility = "//visibility:public",
)

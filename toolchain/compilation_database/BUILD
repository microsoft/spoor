load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_proto_library")

cc_proto_library(
    name = "extra_actions_base_cc_proto",
    deps = [
        "@com_bazelbuild_bazel//src/main/protobuf:extra_actions_base_proto",
    ],
)

proto_library(
    name = "compile_commands_proto",
    srcs = ["compile_commands.proto"],
)

cc_proto_library(
    name = "compile_commands_cc_proto",
    deps = [":compile_commands_proto"],
)

cc_binary(
    name = "extract_compile_command",
    srcs = [
        "extract_compile_command.cc",
        "extract_compile_command.h",
    ],
    copts = ["-Werror"],
    deps = [
        ":compile_commands_cc_proto",
        ":extra_actions_base_cc_proto",
        "//util:flags",
        "//util:result",
        "//util:strings",
        "@com_gflags_gflags//:gflags",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_binary(
    name = "concatenate_compile_commands",
    srcs = [
        "concatenate_compile_commands.cc",
        "concatenate_compile_commands.h",
        "extract_compile_command.h",
    ],
    copts = ["-Werror"],
    deps = [
        ":compile_commands_cc_proto",
        ":extra_actions_base_cc_proto",
        "//util:flags",
        "//util:result",
        "//util:strings",
        "@com_gflags_gflags//:gflags",
        "@com_google_protobuf//:protobuf",
    ],
)

action_listener(
    name = "extract_compile_command_listener",
    extra_actions = [":extract_compile_command_action"],
    mnemonics = ["CppCompile"],
    visibility = ["//visibility:public"],
)

extra_action(
    name = "extract_compile_command_action",
    cmd = "$(location :extract_compile_command) \
        --extra_action_file=$(EXTRA_ACTION_FILE) \
        --output=$(output $(ACTION_ID).compile_command.pb)",
    out_templates = ["$(ACTION_ID).compile_command.pb"],
    tools = [":extract_compile_command"],
)

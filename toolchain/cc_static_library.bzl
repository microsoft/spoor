# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

"""
`cc_static_library` creates a self-contained static library from the rule's
dependencies.
"""

load("@rules_cc//cc:action_names.bzl", "CPP_LINK_STATIC_LIBRARY_ACTION_NAME")
load("@rules_cc//cc:toolchain_utils.bzl", "find_cpp_toolchain")

def _cc_static_library_impl(ctx):
    output_file_name = "lib%s.a" % ctx.label.name
    output_file = ctx.actions.declare_file(output_file_name)

    input_objects = []
    for dep_target in ctx.attr.deps:
        for dep in dep_target[CcInfo].linking_context.linker_inputs.to_list():
            for library in dep.libraries:
                input_objects += library.objects

    cc_toolchain = find_cpp_toolchain(ctx)

    feature_configuration = cc_common.configure_features(
        ctx = ctx,
        cc_toolchain = cc_toolchain,
        requested_features = ctx.features,
        unsupported_features = ctx.disabled_features,
    )

    library = cc_common.create_library_to_link(
        actions = ctx.actions,
        feature_configuration = feature_configuration,
        cc_toolchain = cc_toolchain,
        static_library = output_file,
    )
    linker_input = cc_common.create_linker_input(
        owner = ctx.label,
        libraries = depset(direct = [library]),
    )

    compilation_context = cc_common.create_compilation_context()
    linking_context = cc_common.create_linking_context(
        linker_inputs = depset(direct = [linker_input]),
    )

    archiver_path = cc_common.get_tool_for_action(
        feature_configuration = feature_configuration,
        action_name = CPP_LINK_STATIC_LIBRARY_ACTION_NAME,
    )
    archiver_variables = cc_common.create_link_variables(
        cc_toolchain = cc_toolchain,
        feature_configuration = feature_configuration,
        output_file = output_file.path,
        is_using_linker = False,
    )
    command_line = cc_common.get_memory_inefficient_command_line(
        feature_configuration = feature_configuration,
        action_name = CPP_LINK_STATIC_LIBRARY_ACTION_NAME,
        variables = archiver_variables,
    )

    arguments = ctx.actions.args()
    arguments.add_all(command_line)
    arguments.add_all([obj.path for obj in input_objects])

    env = cc_common.get_environment_variables(
        feature_configuration = feature_configuration,
        action_name = CPP_LINK_STATIC_LIBRARY_ACTION_NAME,
        variables = archiver_variables,
    )

    ctx.actions.run(
        outputs = [output_file],
        inputs = depset(
            direct = input_objects,
            transitive = [cc_toolchain.all_files],
        ),
        executable = archiver_path,
        arguments = [arguments],
        mnemonic = "StaticArchive",
        progress_message = "Creating static archive %s" % output_file_name,
        env = env,
    )

    default_info = DefaultInfo(files = depset(direct = [output_file]))
    this_cc_info = CcInfo(
        compilation_context = compilation_context,
        linking_context = linking_context,
    )
    deps_cc_infos = [dep[CcInfo] for dep in ctx.attr.deps]
    cc_info = cc_common.merge_cc_infos(
        cc_infos = [this_cc_info] + deps_cc_infos,
    )
    return [default_info, cc_info]

cc_static_library = rule(
    implementation = _cc_static_library_impl,
    attrs = {
        "deps": attr.label_list(providers = [CcInfo]),
        "_cc_toolchain": attr.label(
            default = Label("@bazel_tools//tools/cpp:current_cc_toolchain"),
        ),
    },
    fragments = ["cpp"],
    toolchains = ["@bazel_tools//tools/cpp:toolchain_type"],
    incompatible_use_toolchain_transition = True,
    doc = "Create a self-contained static library from the dependencies.",
)

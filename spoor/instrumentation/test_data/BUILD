# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

filegroup(
    name = "test_data",
    testonly = True,
    srcs = [
        "fib.ll",
        "fib_debug_c.ll",
        "fib_debug_cpp.ll",
        "fib_debug_objc.ll",
        "fib_debug_swift.ll",
        "fib_instrumented.ll",
        "fib_instrumented_initialized.ll",
        "fib_instrumented_initialized_enabled.ll",
        "fib_no_main.ll",
        "fib_only_main_instrumented.ll",
        "main_difile_compiler_generated.ll",
        "main_difile_empty_directory.ll",
        "main_difile_file_name_contains_directory.ll",
        "main_disubprogram_zero_line_number.ll",
    ],
    visibility = ["//spoor/instrumentation:__subpackages__"],
)

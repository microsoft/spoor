# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

sh_test(
    name = "integration_test",
    size = "small",
    srcs = ["integration_test.sh"],
    data = [
        "//spoor/instrumentation:spoor_opt",
        "//spoor/runtime:spoor_runtime",
        "//spoor/runtime:spoor_runtime_default_config",
        "//spoor/test_data",
        "//spoor/tools:spoor",
        "@dev_perfetto_trace_processor//file:trace_processor",
    ],
    visibility = ["//visibility:private"],
)

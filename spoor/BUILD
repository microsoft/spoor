# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

sh_test(
    name = "integration_test",
    size = "small",
    srcs = ["integration_test.sh"],
    data = [
        "//spoor/instrumentation:spoor_opt",
        "//spoor/runtime:spoor_runtime",
        "//spoor/test_data",
    ],
    visibility = ["//visibility:private"],
)

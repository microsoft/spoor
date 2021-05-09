# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

SPOOR_DEFAULT_COPTS = [
    "-fno-exceptions",
    "-fno-rtti",
    "-Wall",
    "-Wextra",
    "-pedantic",
    "-Werror",
    "-Wno-gcc-compat",  # absl::StrFormat
]

SPOOR_DEFAULT_LINKOPTS = [
    "-lc++",
    "-lstdc++",
]

SPOOR_DEFAULT_TEST_COPTS = SPOOR_DEFAULT_COPTS + [
    "-Wno-gnu-zero-variadic-macro-arguments",  # gMock
]

SPOOR_DEFAULT_TEST_LINKOPTS = SPOOR_DEFAULT_LINKOPTS

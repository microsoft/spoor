// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <cstdint>

using int8 = int8_t;      // NOLINT(readability-identifier-naming)
using uint8 = uint8_t;    // NOLINT(readability-identifier-naming)
using int16 = int16_t;    // NOLINT(readability-identifier-naming)
using uint16 = uint16_t;  // NOLINT(readability-identifier-naming)
using int32 = int32_t;    // NOLINT(readability-identifier-naming)
using uint32 = uint32_t;  // NOLINT(readability-identifier-naming)
using int64 = int64_t;    // NOLINT(readability-identifier-naming)
using uint64 = uint64_t;  // NOLINT(readability-identifier-naming)

static_assert(
    sizeof(float) == 4,
    "This platform does not define `float` as a 32-bit floating-point value.");
static_assert(
    sizeof(double) == 8,
    "This platform does not define `double` as a 64-bit floating-point value.");

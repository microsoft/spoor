#pragma once

#include <cstdint>
#include <type_traits>

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

static_assert(std::is_integral_v<int8>);
static_assert(std::is_signed_v<int8>);
static_assert(sizeof(int8) == 1);

static_assert(std::is_integral_v<uint8>);
static_assert(!std::is_signed_v<uint8>);
static_assert(sizeof(uint8) == 1);

static_assert(std::is_integral_v<int16>);
static_assert(std::is_signed_v<int16>);
static_assert(sizeof(int16) == 2);

static_assert(std::is_integral_v<uint16>);
static_assert(!std::is_signed_v<uint16>);
static_assert(sizeof(uint16) == 2);

static_assert(std::is_integral_v<int32>);
static_assert(std::is_signed_v<int32>);
static_assert(sizeof(int32) == 4);

static_assert(std::is_integral_v<uint32>);
static_assert(!std::is_signed_v<uint32>);
static_assert(sizeof(uint32) == 4);

static_assert(std::is_integral_v<int64>);
static_assert(std::is_signed_v<int64>);
static_assert(sizeof(int64) == 8);

static_assert(std::is_integral_v<uint64>);
static_assert(!std::is_signed_v<uint64>);
static_assert(sizeof(uint64) == 8);

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "gsl/gsl"
#include "util/numeric.h"
#include "util/result.h"

namespace util::compression {

enum class Strategy : uint8 {
  kNone = 0,
  kSnappy = 1,
};

constexpr std::array<Strategy, 2> kStrategies{
    {Strategy::kNone, Strategy::kSnappy}};

class Compressor {
 public:
  enum class CompressError {};

  enum class UncompressError {
    kCorruptInput,
  };

  using CompressResult =
      util::result::Result<gsl::span<const char>, CompressError>;
  using UncompressResult =
      util::result::Result<gsl::span<const char>, UncompressError>;
  using SizeType = gsl::span<const char>::size_type;

  constexpr Compressor() = default;
  constexpr Compressor(const Compressor&) = default;
  constexpr Compressor(Compressor&&) = default;
  constexpr auto operator=(const Compressor&) -> Compressor& = default;
  constexpr auto operator=(Compressor&&) noexcept -> Compressor& = default;
  virtual constexpr ~Compressor() = default;

  [[nodiscard]] virtual auto Strategy() const -> Strategy = 0;
  [[nodiscard]] virtual auto Compress(gsl::span<const char> uncompressed_data)
      -> CompressResult = 0;
  [[nodiscard]] virtual auto Uncompress(gsl::span<const char> compressed_data)
      -> UncompressResult = 0;
};

}  // namespace util::compression

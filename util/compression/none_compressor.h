// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "gsl/gsl"
#include "util/compression/compressor.h"

namespace util::compression {

class NoneCompressor final : public Compressor {
 public:
  constexpr NoneCompressor() = default;
  constexpr NoneCompressor(const NoneCompressor&) = default;
  constexpr NoneCompressor(NoneCompressor&&) noexcept = default;
  constexpr auto operator=(const NoneCompressor&) -> NoneCompressor& = default;
  constexpr auto operator=(NoneCompressor&&) noexcept
      -> NoneCompressor& = default;
  constexpr ~NoneCompressor() override = default;

  [[nodiscard]] auto Strategy() const -> enum Strategy override;
  [[nodiscard]] auto Compress(gsl::span<const char> uncompressed_data)
      -> CompressResult override;
  [[nodiscard]] auto Uncompress(gsl::span<const char> compressed_data)
      -> UncompressResult override;
};

}  // namespace util::compression

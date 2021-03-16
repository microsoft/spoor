// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <string>

#include "gsl/gsl"
#include "util/compression/compressor.h"

namespace util::compression {

class SnappyCompressor final : public Compressor {
 public:
  constexpr SnappyCompressor() = delete;
  explicit SnappyCompressor(SizeType initial_capacity);
  SnappyCompressor(const SnappyCompressor&) = default;
  SnappyCompressor(SnappyCompressor&&) noexcept = default;
  auto operator=(const SnappyCompressor&) -> SnappyCompressor& = default;
  auto operator=(SnappyCompressor&&) noexcept -> SnappyCompressor& = default;
  ~SnappyCompressor() override = default;

  [[nodiscard]] auto Strategy() const -> enum Strategy override;
  [[nodiscard]] auto Compress(gsl::span<const char> uncompressed_data)
      -> CompressResult override;
  [[nodiscard]] auto Uncompress(gsl::span<const char> compressed_data)
      -> UncompressResult override;

 private:
  std::string buffer_;
};

}  // namespace util::compression

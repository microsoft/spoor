// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/compression/snappy_compressor.h"

#include "gsl/gsl"
#include "util/compression/compressor.h"
#include "util/compression/snappy/include/snappy.h"

namespace util::compression {

SnappyCompressor::SnappyCompressor(const SizeType initial_capacity) {
  buffer_.reserve(initial_capacity);
}

auto SnappyCompressor::Strategy() const -> enum Strategy {
  return Strategy::kSnappy;
}

auto SnappyCompressor::Compress(const gsl::span<const char> uncompressed_data)
    -> CompressResult {
  const auto compressed_size = snappy::Compress(
      uncompressed_data.data(), uncompressed_data.size(), &buffer_);
  return gsl::span<const char>{buffer_.data(), compressed_size};
}

auto SnappyCompressor::Uncompress(const gsl::span<const char> compressed_data)
    -> UncompressResult {
  const auto success = snappy::Uncompress(compressed_data.data(),
                                          compressed_data.size(), &buffer_);
  if (!success) return UncompressError::kCorruptInput;
  return gsl::span<const char>{buffer_.data(), buffer_.size()};
}

}  // namespace util::compression

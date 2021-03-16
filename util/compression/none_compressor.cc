// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/compression/none_compressor.h"

#include "gsl/gsl"
#include "util/compression/compressor.h"

namespace util::compression {

auto NoneCompressor::Strategy() const -> enum Strategy {
  return Strategy::kNone;
}

auto NoneCompressor::Compress(gsl::span<const char> uncompressed_data)
    -> CompressResult {
  return uncompressed_data;
}

auto NoneCompressor::Uncompress(gsl::span<const char> compressed_data)
    -> UncompressResult {
  return compressed_data;
}

}  // namespace util::compression

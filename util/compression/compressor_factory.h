// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <memory>

#include "util/compression/compressor.h"

namespace util::compression {

auto MakeCompressor(Strategy strategy, Compressor::SizeType initial_capacity)
    -> std::unique_ptr<Compressor>;

}  // namespace util::compression

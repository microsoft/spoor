// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/compression/compressor_factory.h"

#include <memory>
// #include <iostream>

#include "util/compression/compressor.h"
#include "util/compression/none_compressor.h"
#include "util/compression/snappy_compressor.h"

namespace util::compression {

auto MakeCompressor(const Strategy strategy,
                    const Compressor::SizeType initial_capacity)
    -> std::unique_ptr<Compressor> {
  // switch (strategy) {
  //   case Strategy::kNone: {
  //     std::cout << "make compressor: none\n";
  //     break;
  //   }
  //   case Strategy::kSnappy: {
  //     std::cout << "make compressor: snappy\n";
  //     break;
  //   }
  // }
  switch (strategy) {
    case Strategy::kNone:
      return std::make_unique<NoneCompressor>();
    case Strategy::kSnappy:
      return std::make_unique<SnappyCompressor>(initial_capacity);
  }
}

}  // namespace util::compression

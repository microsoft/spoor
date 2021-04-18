// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/compression/compressor.h"

#include <array>
#include <memory>
#include <string_view>
#include <vector>

#include "gsl/gsl"
#include "gtest/gtest.h"
#include "util/compression/compressor_factory.h"
#include "util/compression/none_compressor.h"
#include "util/compression/snappy_compressor.h"

namespace {

using util::compression::Compressor;
using util::compression::kStrategies;
using util::compression::MakeCompressor;
using util::compression::NoneCompressor;
using util::compression::SnappyCompressor;
using SizeType = util::compression::Compressor::SizeType;

constexpr std::array<std::string_view, 4> kTestData{
    {"", "abc", "xxxxxxxxxxxxxxxxx", "Hello, world!"}};

auto MakeCompressors(const SizeType initial_capacity)
    -> std::vector<std::unique_ptr<Compressor>> {
  std::vector<std::unique_ptr<Compressor>> result{};
  result.reserve(kStrategies.size());
  std::transform(std::cbegin(kStrategies), std::cend(kStrategies),
                 std::back_inserter(result),
                 [initial_capacity](const auto strategy) {
                   return MakeCompressor(strategy, initial_capacity);
                 });
  return result;
}

TEST(Compressor, CompressorFactory) {  // NOLINT
  for (const auto strategy : kStrategies) {
    const auto compressor = MakeCompressor(strategy, 0);
    ASSERT_EQ(compressor->Strategy(), strategy);
  }
}

TEST(Compressor, CompressesAndUncompressesData) {  // NOLINT
  for (auto& compressor : MakeCompressors(0)) {
    for (const auto& original_data : kTestData) {
      const gsl::span<const char> original_span{original_data.data(),
                                                original_data.size()};
      auto const compress_result =
          compressor->Compress({original_data.data(), original_data.size()});
      ASSERT_TRUE(compress_result.IsOk());
      const std::string compressed_data{compress_result.Ok().data(),
                                        compress_result.Ok().size()};
      const auto uncompress_result = compressor->Uncompress(
          {compressed_data.data(), compressed_data.size()});
      ASSERT_TRUE(uncompress_result.IsOk());
      const auto uncompressed_data = uncompress_result.Ok();
      ASSERT_TRUE(compress_result.IsOk());
      ASSERT_EQ(original_span, uncompressed_data);
    }
  }
}

TEST(NoneCompressor, CompressedIsUncompressed) {  // NOLINT
  NoneCompressor compressor{};
  for (const auto& original_data : kTestData) {
    const gsl::span<const char> original_span{original_data.data(),
                                              original_data.size()};
    const auto compressed_result =
        compressor.Compress({original_data.data(), original_data.size()});
    ASSERT_TRUE(compressed_result.IsOk());
    const auto compressed_data = compressed_result.Ok();
    ASSERT_EQ(original_span, compressed_data);
  }
}

TEST(SnappyCompressor, CompressesData) {  // NOLINT
  // NOTE: Compressed Snappy data might be larger than the original. The test
  // cases below are cherry-picked examples that are known to work.
  constexpr std::string_view abc{"abcabcabcabcabcabcabcabcabcabcabcabc"};
  constexpr std::string_view x{"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"};
  SnappyCompressor compressor{0};
  for (const auto& original_data : {abc, x}) {
    const auto compressed_result =
        compressor.Compress({original_data.data(), original_data.size()});
    ASSERT_TRUE(compressed_result.IsOk());
    const auto compressed_data = compressed_result.Ok();
    ASSERT_LT(compressed_data.size(), original_data.size());
  }
}

}  // namespace

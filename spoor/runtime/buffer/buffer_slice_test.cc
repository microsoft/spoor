// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <memory>
#include <numeric>
#include <vector>

#include "gsl/gsl"
#include "gtest/gtest.h"
#include "spoor/runtime/buffer/circular_buffer.h"
#include "spoor/runtime/buffer/owned_buffer_slice.h"
#include "spoor/runtime/buffer/unowned_buffer_slice.h"
#include "util/numeric.h"

namespace {

using BufferSlice = spoor::runtime::buffer::CircularBuffer<uint64>;
using ValueType = BufferSlice::ValueType;
using OwnedBufferSlice = spoor::runtime::buffer::OwnedBufferSlice<ValueType>;
using UnownedBufferSlice =
    spoor::runtime::buffer::UnownedBufferSlice<ValueType>;
using SizeType = BufferSlice::SizeType;

auto Slices(gsl::span<ValueType> buffer)
    -> std::vector<std::unique_ptr<BufferSlice>> {
  std::vector<std::unique_ptr<BufferSlice>> slices{};
  slices.reserve(2);
  slices.emplace_back(std::make_unique<OwnedBufferSlice>(buffer.size()));
  slices.emplace_back(std::make_unique<UnownedBufferSlice>(buffer));
  return slices;
}

TEST(BufferSlice, ContiguousMemoryChunksOneChunk) {  // NOLINT
  constexpr SizeType capacity{5};
  std::vector<ValueType> buffer(capacity);
  for (auto& slice : Slices(buffer)) {
    ASSERT_TRUE(slice->ContiguousMemoryChunks().empty());
    std::vector<ValueType> expected{};
    for (SizeType i{0}; i < capacity; ++i) {
      slice->Push(i);
      expected.emplace_back(i);
      const auto chunks = slice->ContiguousMemoryChunks();
      ASSERT_EQ(chunks.size(), 1);
      auto chunk = chunks.front();
      ASSERT_EQ(chunk.size(), expected.size());
      ASSERT_TRUE(std::equal(std::cbegin(chunk), std::cend(chunk),
                             std::cbegin(expected)));
    }
    for (SizeType i{capacity}; i < 5 * capacity; ++i) {
      slice->Push(i);
      if ((i + 1) % capacity == 0) {
        const auto chunks = slice->ContiguousMemoryChunks();
        ASSERT_EQ(chunks.size(), 1);
        const auto chunk = chunks.front();
        ASSERT_EQ(chunk.size(), capacity);
        std::vector<ValueType> expected(capacity);
        std::iota(std::begin(expected), std::end(expected), i - capacity + 1);
        ASSERT_TRUE(std::equal(std::cbegin(chunk), std::cend(chunk),
                               std::cbegin(expected)));
      }
    }
  }
}

TEST(BufferSlice, ContiguousMemoryChunksTwoChunks) {  // NOLINT
  constexpr SizeType capacity{5};
  std::vector<ValueType> buffer(capacity);
  for (auto& slice : Slices(buffer)) {
    std::vector<ValueType> expected{};
    for (SizeType i{0}; i < capacity; ++i) {
      slice->Push(i);
    }
    for (SizeType i{capacity}; i < 3 * capacity; ++i) {
      slice->Push(i);
      if ((i + 1) % capacity != 0) {
        const auto chunks = slice->ContiguousMemoryChunks();
        ASSERT_EQ(chunks.size(), 2);
        const auto first_chunk = chunks.front();
        const auto expected_first_chunk_size = (capacity - (i + 1) % capacity);
        ASSERT_EQ(first_chunk.size(), expected_first_chunk_size);
        std::vector<ValueType> expected_first_chunk(expected_first_chunk_size);
        std::iota(std::begin(expected_first_chunk),
                  std::end(expected_first_chunk), i - capacity + 1);
        ASSERT_EQ(first_chunk.size(), expected_first_chunk.size());
        ASSERT_TRUE(std::equal(std::cbegin(first_chunk), std::cend(first_chunk),
                               std::cbegin(expected_first_chunk)));
        const auto second_chunk = chunks.back();
        const auto expected_second_chunk_size = ((i + 1) % capacity);
        ASSERT_EQ(second_chunk.size(), expected_second_chunk_size);
        std::vector<ValueType> expected_second_chunk(
            expected_second_chunk_size);
        std::iota(std::begin(expected_second_chunk),
                  std::end(expected_second_chunk),
                  i - capacity + 1 + expected_first_chunk_size);
        ASSERT_EQ(second_chunk.size(), expected_second_chunk.size());
        ASSERT_TRUE(std::equal(std::cbegin(second_chunk),
                               std::cend(second_chunk),
                               std::cbegin(expected_second_chunk)));
      }
    }
  }
}

}  // namespace

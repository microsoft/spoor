#include "spoor/runtime/buffer/buffer_slice.h"

#include <algorithm>
#include <memory>
#include <numeric>
#include <span>
#include <utility>

#include "gtest/gtest.h"
#include "spoor/runtime/buffer/owned_buffer_slice.h"
#include "spoor/runtime/buffer/unowned_buffer_slice.h"
#include "util/numeric.h"

namespace {

using BufferSlice = spoor::runtime::buffer::BufferSlice<int64>;
using ValueType = BufferSlice::ValueType;
using OwnedBufferSlice = spoor::runtime::buffer::OwnedBufferSlice<ValueType>;
using UnownedBufferSlice =
    spoor::runtime::buffer::UnownedBufferSlice<ValueType>;
using SizeType = BufferSlice::SizeType;

auto Slices(std::span<ValueType> buffer)
    -> std::vector<std::unique_ptr<BufferSlice>> {
  std::vector<std::unique_ptr<BufferSlice>> slices{};
  slices.reserve(2);
  slices.push_back(std::make_unique<UnownedBufferSlice>(buffer));
  slices.push_back(std::make_unique<OwnedBufferSlice>(buffer.size()));
  return slices;
}

TEST(BufferSlice, Clear) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    std::vector<ValueType> buffer(capacity);
    for (auto& slice : Slices(buffer)) {
      for (SizeType i{0}; i < capacity; ++i) {
        slice->Push(i);
      }
      slice->Clear();
      ASSERT_EQ(slice->Size(), 0);
    }
  }
}

TEST(BufferSlice, Size) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    std::vector<ValueType> buffer(capacity);
    for (auto& slice : Slices(buffer)) {
      for (SizeType i{0}; i < 2 * capacity; ++i) {
        slice->Push(i);
        ASSERT_EQ(slice->Size(), std::min(i + 1, capacity));
      }
    }
  }
}

TEST(BufferSlice, Capacity) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    std::vector<ValueType> buffer(capacity);
    for (auto& slice : Slices(buffer)) {
      for (SizeType i{0}; i < 2 * capacity; ++i) {
        slice->Push(i);
        ASSERT_EQ(slice->Capacity(), capacity);
      }
    }
  }
}

TEST(BufferSlice, Empty) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    std::vector<ValueType> buffer(capacity);
    for (auto& slice : Slices(buffer)) {
      ASSERT_TRUE(slice->Empty());
      for (SizeType i{0}; i < capacity; ++i) {
        slice->Push(i);
        ASSERT_FALSE(slice->Empty());
      }
    }
  }
}

TEST(BufferSlice, WillWrapOnNextPush) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    std::vector<ValueType> buffer(capacity);
    for (auto& slice : Slices(buffer)) {
      std::cerr << "capacity = " << capacity << '\n';
      std::cerr << "size = " << slice->Size() << '\n';
      if (capacity == 0) ASSERT_TRUE(slice->WillWrapOnNextPush());
      for (SizeType i{0}; i < 5 * capacity; ++i) {
        if ((i + 1) % capacity == 0) {
          ASSERT_TRUE(slice->WillWrapOnNextPush());
        } else {
          ASSERT_FALSE(slice->WillWrapOnNextPush());
        }
        slice->Push(i);
        if ((i + 1) % capacity != 0 && capacity < i + 1) {
          ASSERT_EQ(slice->ContiguousMemoryChunks().size(), 2);
        } else {
          ASSERT_EQ(slice->ContiguousMemoryChunks().size(), 1);
        }
      }
    }
  }
}

TEST(BufferSlice, ContiguousMemoryChunksEmpty) {  // NOLINT
  std::vector<ValueType> buffer(0);
  for (auto& slice : Slices(buffer)) {
    ASSERT_TRUE(slice->ContiguousMemoryChunks().empty());
  }
}

TEST(BufferSlice, ContiguousMemoryChunksOneChunk) {  // NOLINT
  const SizeType capacity{5};
  std::vector<ValueType> buffer(capacity);
  for (auto& slice : Slices(buffer)) {
    ASSERT_TRUE(slice->ContiguousMemoryChunks().empty());

    std::vector<ValueType> expected{};
    for (SizeType i{0}; i < capacity; ++i) {
      slice->Push(i);
      expected.push_back(i);
      const auto chunks = slice->ContiguousMemoryChunks();
      ASSERT_EQ(chunks.size(), 1);
      auto chunk = chunks.front();
      ASSERT_EQ(chunk.size(), expected.size());
      ASSERT_TRUE(std::equal(chunk.cbegin(), chunk.cend(), expected.cbegin()));
    }
    for (SizeType i{capacity}; i < 5 * capacity; ++i) {
      slice->Push(i);
      if ((i + 1) % capacity == 0) {
        const auto chunks = slice->ContiguousMemoryChunks();
        ASSERT_EQ(chunks.size(), 1);
        const auto chunk = chunks.front();
        ASSERT_EQ(chunk.size(), capacity);
        std::vector<ValueType> expected(capacity);
        std::iota(expected.begin(), expected.end(), i - capacity + 1);
        ASSERT_TRUE(
            std::equal(chunk.cbegin(), chunk.cend(), expected.cbegin()));
      }
    }
  }
}

TEST(BufferSlice, ContiguousMemoryChunksTwoChunks) {  // NOLINT
  const SizeType capacity{5};
  std::vector<ValueType> buffer(capacity);
  for (auto& slice : Slices(buffer)) {
    std::vector<ValueType> expected{};
    for (SizeType i{0}; i < capacity; ++i) {
      slice->Push(i);
    }
    for (SizeType i{capacity}; i < 5 * capacity; ++i) {
      slice->Push(i);
      if ((i + 1) % capacity != 0) {
        const auto chunks = slice->ContiguousMemoryChunks();
        ASSERT_EQ(chunks.size(), 2);

        const auto first_chunk = chunks.front();
        const auto expected_first_chunk_size = (capacity - (i + 1) % capacity);
        ASSERT_EQ(first_chunk.size(), expected_first_chunk_size);
        std::vector<ValueType> expected_first_chunk(expected_first_chunk_size);
        std::iota(expected_first_chunk.begin(), expected_first_chunk.end(),
                  i - capacity + 1);
        ASSERT_TRUE(std::equal(first_chunk.cbegin(), first_chunk.cend(),
                               expected_first_chunk.cbegin()));

        const auto second_chunk = chunks.back();
        const auto expected_second_chunk_size = ((i + 1) % capacity);
        ASSERT_EQ(second_chunk.size(), expected_second_chunk_size);
        std::vector<ValueType> expected_second_chunk(
            expected_second_chunk_size);
        std::iota(expected_second_chunk.begin(), expected_second_chunk.end(),
                  i - capacity + 1 + expected_first_chunk_size);
        ASSERT_TRUE(std::equal(second_chunk.cbegin(), second_chunk.cend(),
                               expected_second_chunk.cbegin()));
      }
    }
  }
}

}  // namespace

#include "spoor/runtime/buffer/buffer_slice.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <numeric>
#include <type_traits>
#include <utility>

#include "spoor/runtime/buffer/buffer_slice_pool.h"
#include "spoor/runtime/buffer/contiguous_memory.h"
#include "util/numeric.h"

namespace {

using BufferSlice = spoor::runtime::buffer::BufferSlice<int64>;
using ValueType = BufferSlice::ValueType;
using BufferSlicePool = spoor::runtime::buffer::BufferSlicePool<ValueType>;
using ContiguousMemory = spoor::runtime::buffer::ContiguousMemory<ValueType>;
using SizeType = BufferSlice::SizeType;

TEST(BufferSlice, Size) {  // NOLINT
  for (const SizeType size : {0, 1, 2, 10}) {
    const BufferSlice::Options slice_options{.capacity = size};
    const BufferSlicePool::Options pool_options{.reserved_pool_capacity = 1,
                                                .dynamic_pool_capacity = 0};
    BufferSlicePool pool{pool_options, slice_options};
    auto slice = pool.Borrow();
    for (SizeType i{0}; i < 2 * size; ++i) {
      slice->Push(i);
      ASSERT_EQ(slice->Size(), std::min(i + 1, size));
    }
    pool.Return(std::move(slice));
  }
}

TEST(BufferSlice, Capacity) {  // NOLINT
  for (const SizeType size : {0, 1, 2, 10}) {
    const BufferSlice::Options slice_options{.capacity = size};
    const BufferSlicePool::Options pool_options{.reserved_pool_capacity = 1,
                                                .dynamic_pool_capacity = 0};
    BufferSlicePool pool{pool_options, slice_options};
    auto slice = pool.Borrow();
    for (SizeType i{0}; i < 2 * size; ++i) {
      slice->Push(i);
      ASSERT_EQ(slice->Capacity(), size);
    }
    pool.Return(std::move(slice));
  }
}

TEST(BufferSlice, Empty) {  // NOLINT
  for (const SizeType size : {0, 1, 2, 10}) {
    const BufferSlice::Options slice_options{.capacity = size};
    const BufferSlicePool::Options pool_options{.reserved_pool_capacity = 1,
                                                .dynamic_pool_capacity = 0};
    BufferSlicePool pool{pool_options, slice_options};
    auto slice = pool.Borrow();
    ASSERT_TRUE(slice->Empty());
    for (SizeType i{0}; i < size; ++i) {
      slice->Push(i);
      ASSERT_FALSE(slice->Empty());
    }
    pool.Return(std::move(slice));
  }
}

TEST(BufferSlice, ContiguousMemoryChunksEmpty) {  // NOLINT
  const BufferSlice::Options slice_options{.capacity = 3};
  const BufferSlicePool::Options pool_options{.reserved_pool_capacity = 1,
                                              .dynamic_pool_capacity = 0};
  BufferSlicePool pool{pool_options, slice_options};
  auto slice = pool.Borrow();
  const std::vector<ContiguousMemory> empty{};
  ASSERT_EQ(slice->ContiguousMemoryChunks(), empty);
  pool.Return(std::move(slice));
}

TEST(BufferSlice, ContiguousMemoryChunksOneChunk) {  // NOLINT
  const SizeType buffer_size{5};
  const BufferSlice::Options slice_options{.capacity = buffer_size};
  const BufferSlicePool::Options pool_options{.reserved_pool_capacity = 1,
                                              .dynamic_pool_capacity = 0};
  BufferSlicePool pool{pool_options, slice_options};
  auto slice = pool.Borrow();

  const std::vector<ContiguousMemory> empty{};
  ASSERT_EQ(slice->ContiguousMemoryChunks(), empty);

  std::vector<ValueType> expected{};
  for (SizeType i{0}; i < buffer_size; ++i) {
    slice->Push(i);
    expected.push_back(i);
    const auto chunks = slice->ContiguousMemoryChunks();
    ASSERT_EQ(chunks.size(), 1);
    auto chunk = *chunks.begin();
    ASSERT_EQ(chunk.size, (i + 1) * sizeof(ValueType));
    ASSERT_EQ(std::memcmp(chunk.begin, expected.data(), chunk.size), 0);
  }
  for (SizeType i{buffer_size}; i < 5 * buffer_size; ++i) {
    slice->Push(i);
    if ((i + 1) % buffer_size == 0) {
      const auto chunks = slice->ContiguousMemoryChunks();
      ASSERT_EQ(chunks.size(), 1);
      const auto chunk = *chunks.begin();
      ASSERT_EQ(chunk.size, buffer_size * sizeof(ValueType));
      std::vector<ValueType> expected(buffer_size);
      std::iota(expected.begin(), expected.end(), i - buffer_size + 1);
      ASSERT_EQ(std::memcmp(chunk.begin, expected.data(), chunk.size), 0);
    }
  }
  pool.Return(std::move(slice));
}

TEST(BufferSlice, ContiguousMemoryChunksTwoChunks) {  // NOLINT
  const SizeType buffer_size{5};
  const BufferSlice::Options slice_options{.capacity = buffer_size};
  const BufferSlicePool::Options pool_options{.reserved_pool_capacity = 1,
                                              .dynamic_pool_capacity = 0};
  BufferSlicePool pool{pool_options, slice_options};
  auto slice = pool.Borrow();

  std::vector<ValueType> expected{};
  for (SizeType i{0}; i < buffer_size; ++i) {
    slice->Push(i);
  }
  for (SizeType i{buffer_size}; i < 5 * buffer_size; ++i) {
    slice->Push(i);
    if ((i + 1) % buffer_size != 0) {
      const auto chunks = slice->ContiguousMemoryChunks();
      ASSERT_EQ(chunks.size(), 2);

      const auto first_chunk = *chunks.begin();
      const auto expected_first_chunk_size =
          (buffer_size - (i + 1) % buffer_size);
      ASSERT_EQ(first_chunk.size,
                expected_first_chunk_size * sizeof(ValueType));
      std::vector<ValueType> expected_first_chunk(expected_first_chunk_size);
      std::iota(expected_first_chunk.begin(), expected_first_chunk.end(),
                i - buffer_size + 1);
      ASSERT_EQ(std::memcmp(first_chunk.begin, expected_first_chunk.data(),
                            first_chunk.size),
                0);

      const auto second_chunk = *std::prev(chunks.end());
      const auto expected_second_chunk_size = ((i + 1) % buffer_size);
      ASSERT_EQ(second_chunk.size,
                expected_second_chunk_size * sizeof(ValueType));
      std::vector<ValueType> expected_second_chunk(expected_second_chunk_size);
      std::iota(expected_second_chunk.begin(), expected_second_chunk.end(),
                i - buffer_size + 1 + expected_first_chunk_size);
      ASSERT_EQ(std::memcmp(second_chunk.begin, expected_second_chunk.data(),
                            second_chunk.size),
                0);
    }
  }
  pool.Return(std::move(slice));
}

TEST(BufferSlice, Clear) {  // NOLINT
  for (const SizeType size : {0, 1, 2, 10}) {
    const BufferSlice::Options slice_options{.capacity = size};
    const BufferSlicePool::Options pool_options{.reserved_pool_capacity = 1,
                                                .dynamic_pool_capacity = 0};
    BufferSlicePool pool{pool_options, slice_options};
    auto slice = pool.Borrow();
    for (SizeType i{0}; i < size; ++i) {
      slice->Push(i);
    }
    slice->Clear();
    ASSERT_EQ(slice->Size(), 0);
    pool.Return(std::move(slice));
  }
}

}  // namespace

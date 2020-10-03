#include "spoor/runtime/buffer/circular_buffer.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "gsl/gsl"
#include "gtest/gtest.h"
#include "spoor/runtime/buffer/circular_slice_buffer.h"
#include "spoor/runtime/buffer/owned_buffer_slice.h"
#include "spoor/runtime/buffer/reserved_buffer_slice_pool.h"
#include "spoor/runtime/buffer/unowned_buffer_slice.h"
#include "util/numeric.h"

namespace {

using CircularBuffer = spoor::runtime::buffer::CircularBuffer<int64>;
using ValueType = CircularBuffer::ValueType;
using SizeType = CircularBuffer::SizeType;
using Pool = spoor::runtime::buffer::ReservedBufferSlicePool<ValueType>;
using OwnedSlicePtr = util::memory::OwnedPtr<CircularBuffer>;

auto MakePool(const SizeType capacity) -> std::unique_ptr<Pool> {
  const typename Pool::Options options{
      .max_slice_capacity = std::max(static_cast<SizeType>(1), capacity / 2),
      .capacity = capacity};
  auto pool = std::make_unique<Pool>(options);
  return pool;
}

template <class T>
auto MakeBuffers(
    std::span<T> buffer,
    gsl::not_null<spoor::runtime::buffer::ReservedBufferSlicePool<T>*> pool)
    -> std::vector<std::unique_ptr<spoor::runtime::buffer::CircularBuffer<T>>> {
  using CircularBuffer = spoor::runtime::buffer::CircularBuffer<T>;
  using OwnedBufferSlice = spoor::runtime::buffer::OwnedBufferSlice<T>;
  using UnownedBufferSlice = spoor::runtime::buffer::UnownedBufferSlice<T>;
  using CircularSliceBuffer = spoor::runtime::buffer::CircularSliceBuffer<T>;
  std::vector<std::unique_ptr<CircularBuffer>> circular_buffers{};
  circular_buffers.reserve(3);
  circular_buffers.push_back(std::make_unique<UnownedBufferSlice>(buffer));
  circular_buffers.push_back(std::make_unique<OwnedBufferSlice>(buffer.size()));
  const typename CircularSliceBuffer::Options circular_slice_buffer_options{
      .buffer_slice_pool = pool, .capacity = pool->Capacity()};
  circular_buffers.push_back(
      std::make_unique<CircularSliceBuffer>(circular_slice_buffer_options));
  return circular_buffers;
}

TEST(CircularBuffer, PushCopyValue) {  // NOLINT
  using Pool = spoor::runtime::buffer::ReservedBufferSlicePool<std::string>;
  const auto assert_chunks_equal_to_expected =
      [](const std::vector<std::span<Pool::ValueType>>& chunks,
         const Pool::SizeType capacity, const int64 start_value) {
        ASSERT_EQ(chunks.size(), 1);
        std::vector<int64> expected_chunk_numbers(capacity);
        std::iota(std::begin(expected_chunk_numbers),
                  std::end(expected_chunk_numbers), start_value);
        std::vector<std::string> expected_chunk{};
        expected_chunk.reserve(expected_chunk_numbers.size());
        std::transform(std::cbegin(expected_chunk_numbers),
                       std::cend(expected_chunk_numbers),
                       std::back_inserter(expected_chunk),
                       [](const auto i) { return std::to_string(i); });
        const auto& chunk = chunks.front();
        ASSERT_EQ(chunk.size(), expected_chunk.size());
        ASSERT_TRUE(std::equal(std::cbegin(chunk), std::cend(chunk),
                               std::cbegin(expected_chunk)));
      };
  for (const SizeType capacity : {1, 2, 10}) {
    std::vector<std::string> buffer(capacity);
    const typename Pool::Options options{.max_slice_capacity = capacity,
                                         .capacity = capacity};
    auto pool = std::make_unique<Pool>(options);
    for (auto& circular_buffer : MakeBuffers<std::string>(buffer, pool.get())) {
      for (SizeType i{0}; i < capacity; ++i) {
        const auto value = std::to_string(i);
        circular_buffer->Push(value);
      }
      ASSERT_TRUE(circular_buffer->Full());
      const auto chunks = circular_buffer->ContiguousMemoryChunks();
      assert_chunks_equal_to_expected(chunks, capacity, 0);

      for (SizeType i{capacity}; i < 2 * capacity; ++i) {
        const auto value = std::to_string(i);
        circular_buffer->Push(value);
      }
      ASSERT_TRUE(circular_buffer->Full());
      const auto wrapped_chunks = circular_buffer->ContiguousMemoryChunks();
      assert_chunks_equal_to_expected(wrapped_chunks, capacity, capacity);
    }
  }
}

TEST(CircularBuffer, PushMoveValue) {  // NOLINT
  const auto assert_chunks_equal_to_expected =
      [](const std::vector<std::span<ValueType>>& chunks,
         const Pool::SizeType capacity, const ValueType start_value) {
        ASSERT_EQ(chunks.size(), 1);
        std::vector<int64> expected_chunk(capacity);
        std::iota(std::begin(expected_chunk),
                  std::end(expected_chunk), start_value);
        const auto& chunk = chunks.front();
        ASSERT_EQ(chunk.size(), expected_chunk.size());
        ASSERT_TRUE(std::equal(std::cbegin(chunk), std::cend(chunk),
                               std::cbegin(expected_chunk)));
      };
  for (const SizeType capacity : {1, 2, 10}) {
    std::vector<ValueType> buffer(capacity);
    const typename Pool::Options options{.max_slice_capacity = capacity,
                                         .capacity = capacity};
    auto pool = std::make_unique<Pool>(options);
    for (auto& circular_buffer : MakeBuffers<ValueType>(buffer, pool.get())) {
      for (SizeType i{0}; i < capacity; ++i) {
        circular_buffer->Push(i);
      }
      ASSERT_TRUE(circular_buffer->Full());
      const auto chunks = circular_buffer->ContiguousMemoryChunks();
      assert_chunks_equal_to_expected(chunks, capacity, 0);

      for (SizeType i{capacity}; i < 2 * capacity; ++i) {
        circular_buffer->Push(i);
      }
      ASSERT_TRUE(circular_buffer->Full());
      const auto wrapped_chunks = circular_buffer->ContiguousMemoryChunks();
      assert_chunks_equal_to_expected(wrapped_chunks, capacity, capacity);
    }
  }
}

TEST(CircularBuffer, Clear) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    std::vector<ValueType> buffer(capacity);
    auto pool = MakePool(capacity);
    for (auto& circular_buffer : MakeBuffers<ValueType>(buffer, pool.get())) {
      for (SizeType i{0}; i < capacity; ++i) {
        circular_buffer->Push(i);
      }
      circular_buffer->Clear();
      ASSERT_EQ(circular_buffer->Size(), 0);
    }
  }
}

TEST(CircularBuffer, Size) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    std::vector<ValueType> buffer(capacity);
    auto pool = MakePool(capacity);
    for (auto& circular_buffer : MakeBuffers<ValueType>(buffer, pool.get())) {
      for (SizeType i{0}; i < 2 * capacity; ++i) {
        circular_buffer->Push(i);
        ASSERT_EQ(circular_buffer->Size(), std::min(i + 1, capacity));
      }
    }
  }
}

TEST(CircularBuffer, Capacity) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    std::vector<ValueType> buffer(capacity);
    auto pool = MakePool(capacity);
    for (auto& circular_buffer : MakeBuffers<ValueType>(buffer, pool.get())) {
      for (SizeType i{0}; i < 2 * capacity; ++i) {
        circular_buffer->Push(i);
        ASSERT_EQ(circular_buffer->Capacity(), capacity);
      }
    }
  }
}

TEST(CircularBuffer, Empty) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    std::vector<ValueType> buffer(capacity);
    auto pool = MakePool(capacity);
    for (auto& circular_buffer : MakeBuffers<ValueType>(buffer, pool.get())) {
      ASSERT_TRUE(circular_buffer->Empty());
      for (SizeType i{0}; i < capacity; ++i) {
        circular_buffer->Push(i);
        ASSERT_FALSE(circular_buffer->Empty());
      }
    }
  }
}

TEST(CircularBuffer, WillWrapOnNextPush) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    std::vector<ValueType> buffer(capacity);
    auto pool = MakePool(capacity);
    for (auto& circular_buffer : MakeBuffers<ValueType>(buffer, pool.get())) {
      for (SizeType i{0}; i < 5 * capacity + 5; ++i) {
        if (capacity == 0 || (i != 0 && i % capacity == 0)) {
          ASSERT_TRUE(circular_buffer->WillWrapOnNextPush());
        } else {
          ASSERT_FALSE(circular_buffer->WillWrapOnNextPush());
        }
        circular_buffer->Push(i);
      }
    }
  }
}

TEST(CircularBuffer, ContiguousMemoryChunksEmpty) {  // NOLINT
  const SizeType capacity{0};
  std::vector<ValueType> buffer(capacity);
  auto pool = MakePool(capacity);
  for (auto& circular_buffer : MakeBuffers<ValueType>(buffer, pool.get())) {
    ASSERT_TRUE(circular_buffer->ContiguousMemoryChunks().empty());
  }
}

}  // namespace

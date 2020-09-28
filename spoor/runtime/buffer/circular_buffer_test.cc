#include "spoor/runtime/buffer/circular_buffer.h"

#include <algorithm>
#include <memory>
#include <span>
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
using CircularSliceBuffer =
    spoor::runtime::buffer::CircularSliceBuffer<ValueType>;
using OwnedBufferSlice = spoor::runtime::buffer::OwnedBufferSlice<ValueType>;
using UnownedBufferSlice =
    spoor::runtime::buffer::UnownedBufferSlice<ValueType>;
using Pool = spoor::runtime::buffer::ReservedBufferSlicePool<ValueType>;
using OwnedSlicePtr = util::memory::OwnedPtr<CircularBuffer>;

auto MakePool(const SizeType capacity) -> std::unique_ptr<Pool> {
  const typename Pool::Options options{
      .max_slice_capacity = std::max(static_cast<SizeType>(1), capacity / 2),
      .capacity = capacity};
  auto pool = std::make_unique<Pool>(options);
  return pool;
}

auto MakeBuffers(std::span<ValueType> buffer, gsl::not_null<Pool*> pool)
    -> std::vector<std::unique_ptr<CircularBuffer>> {
  std::vector<std::unique_ptr<CircularBuffer>> circular_buffers{};
  circular_buffers.reserve(3);
  circular_buffers.push_back(std::make_unique<UnownedBufferSlice>(buffer));
  circular_buffers.push_back(std::make_unique<OwnedBufferSlice>(buffer.size()));
  const auto flush_handler = [](std::vector<OwnedSlicePtr>&& /*unused*/) {};
  const typename CircularSliceBuffer::Options circular_slice_buffer_options{
      .flush_handler = flush_handler,
      .buffer_slice_pool = pool,
      .flush_when_full = false};
  circular_buffers.push_back(
      std::make_unique<CircularSliceBuffer>(circular_slice_buffer_options));
  return circular_buffers;
}

TEST(CircularBuffer, Clear) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    std::vector<ValueType> buffer(capacity);
    auto pool = MakePool(capacity);
    for (auto& circular_buffer : MakeBuffers(buffer, pool.get())) {
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
    for (auto& circular_buffer : MakeBuffers(buffer, pool.get())) {
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
    for (auto& circular_buffer : MakeBuffers(buffer, pool.get())) {
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
    for (auto& circular_buffer : MakeBuffers(buffer, pool.get())) {
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
    for (auto& circular_buffer : MakeBuffers(buffer, pool.get())) {
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
  for (auto& circular_buffer : MakeBuffers(buffer, pool.get())) {
    ASSERT_TRUE(circular_buffer->ContiguousMemoryChunks().empty());
  }
}

}  // namespace

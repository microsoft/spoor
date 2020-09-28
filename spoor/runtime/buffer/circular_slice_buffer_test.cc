#include "spoor/runtime/buffer/circular_slice_buffer.h"

#include <algorithm>
#include <initializer_list>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "spoor/runtime/buffer/reserved_buffer_slice_pool.h"
#include "util/numeric.h"

namespace {

using SlicePool = spoor::runtime::buffer::BufferSlicePool<uint64>;
using CircularBuffer = spoor::runtime::buffer::CircularBuffer<int64>;
using ValueType = CircularBuffer::ValueType;
using SizeType = CircularBuffer::SizeType;
using CircularSliceBuffer =
    spoor::runtime::buffer::CircularSliceBuffer<ValueType>;
using Pool = spoor::runtime::buffer::ReservedBufferSlicePool<ValueType>;
using OwnedSlicePtr = util::memory::OwnedPtr<CircularBuffer>;

// TODO s/size/pushes/
const SizeType kCapacity{10};

auto operator==(const std::vector<std::span<ValueType>>& result_chunks,
                const std::initializer_list<std::initializer_list<ValueType>>&
                    expected_chunks) -> bool {
  if (std::size(result_chunks) != std::size(expected_chunks)) return false;
  for (SizeType index{0}; index < std::size(expected_chunks); ++index) {
    const auto result_chunk = result_chunks.at(index);
    const auto expected_chunk = result_chunks.at(index);
    const auto equal =
        std::equal(std::cbegin(result_chunk), std::cend(result_chunk),
                   std::cbegin(expected_chunk));
    if (!equal) return false;
  }
  return true;
}

auto Print(const std::vector<std::span<ValueType>>& chunks) -> void {
  std::cerr << "test: {";
  for (const auto& chunk : chunks) {
    std::cerr << '{';
    for (const auto& value : chunk) {
      std::cerr << value << ' ';
    }
    std::cerr << '}';
  }
  std::cerr << "}\n";
}

TEST(CircularSliceBuffer, Clear) {  // NOLINT
  const typename Pool::Options options{.max_slice_capacity = 1,
                                       .capacity = kCapacity};
  Pool pool{options};
  ASSERT_EQ(pool.Size(), pool.Capacity());
  const auto flush_handler = [](std::vector<OwnedSlicePtr>&& /*unused*/) {};
  const typename CircularSliceBuffer::Options circular_slice_buffer_options{
      .flush_handler = flush_handler,
      .buffer_slice_pool = &pool,
      .flush_when_full = false};
  CircularSliceBuffer circular_slice_buffer{circular_slice_buffer_options};
  ASSERT_TRUE(circular_slice_buffer.Empty());
  for (SizeType size{0}; size < kCapacity + 1; ++size) {
    for (SizeType i{0}; i < size; ++i) {
      circular_slice_buffer.Push(i);
      ASSERT_EQ(circular_slice_buffer.Size(), i + 1);
    }
    ASSERT_EQ(pool.Size(), pool.Capacity() - circular_slice_buffer.Size());
    circular_slice_buffer.Clear();
    ASSERT_EQ(pool.Size(), pool.Capacity());
    ASSERT_TRUE(circular_slice_buffer.Empty());
  }
}

TEST(CircularSliceBuffer, ManualFlush) {  // NOLINT
  const typename Pool::Options options{.max_slice_capacity = 1,
                                       .capacity = kCapacity};
  Pool pool{options};
  bool flushed{false};
  const auto flush_handler =
      [&flushed](std::vector<OwnedSlicePtr>&& /*unused*/) { flushed = true; };
  const typename CircularSliceBuffer::Options circular_slice_buffer_options{
      .flush_handler = flush_handler,
      .buffer_slice_pool = &pool,
      .flush_when_full = false};
  CircularSliceBuffer circular_slice_buffer{circular_slice_buffer_options};
  for (SizeType size{0}; size < kCapacity + 1; ++size) {
    ASSERT_TRUE(circular_slice_buffer.Empty());
    flushed = false;
    for (SizeType i{0}; i < size; ++i) {
      circular_slice_buffer.Push(i);
      ASSERT_EQ(circular_slice_buffer.Size(), i + 1);
      ASSERT_EQ(pool.Size(), pool.Capacity() - circular_slice_buffer.Size());
    }
    const auto was_empty = circular_slice_buffer.Empty();
    circular_slice_buffer.Flush();
    ASSERT_NE(was_empty, flushed);
    ASSERT_TRUE(pool.Full());
  }
}

TEST(CircularSliceBuffer, AutomaticFlush) {  // NOLINT
  const typename Pool::Options options{.max_slice_capacity = 2,
                                       .capacity = kCapacity};
  Pool pool{options};
  bool flushed{false};
  const auto flush_handler =
      [&flushed](std::vector<OwnedSlicePtr>&& /*unused*/) { flushed = true; };
  const typename CircularSliceBuffer::Options circular_slice_buffer_options{
      .flush_handler = flush_handler,
      .buffer_slice_pool = &pool,
      .flush_when_full = true};
  CircularSliceBuffer circular_slice_buffer{circular_slice_buffer_options};
  ASSERT_TRUE(circular_slice_buffer.Empty());
  for (SizeType i{0}; i < kCapacity * 5; ++i) {
    circular_slice_buffer.Push(i);
    ASSERT_EQ(circular_slice_buffer.Size(), (i + 1) % kCapacity);
    ASSERT_EQ((i + 1) % kCapacity == 0, flushed);
    if (flushed) flushed = false;
  }
}

TEST(CircularSliceBuffer, ContiguousMemoryChunksNoFlushWhenEmpty) {  // NOLINT
  const typename Pool::Options options{.max_slice_capacity = 2,
                                       .capacity = kCapacity};
  Pool pool{options};
  const auto flush_handler = [](std::vector<OwnedSlicePtr>&& /*unused*/) {};
  const typename CircularSliceBuffer::Options circular_slice_buffer_options{
      .flush_handler = flush_handler,
      .buffer_slice_pool = &pool,
      .flush_when_full = false};
  CircularSliceBuffer circular_slice_buffer{circular_slice_buffer_options};

  ASSERT_TRUE(circular_slice_buffer.Empty());
  ASSERT_TRUE(circular_slice_buffer.ContiguousMemoryChunks().empty());
  ASSERT_TRUE(operator==(circular_slice_buffer.ContiguousMemoryChunks(), {}));
  circular_slice_buffer.Push(0);
  ASSERT_TRUE(operator==
              (circular_slice_buffer.ContiguousMemoryChunks(), {{0}}));
  circular_slice_buffer.Push(1);
  ASSERT_TRUE(operator==
              (circular_slice_buffer.ContiguousMemoryChunks(), {{0, 1}}));
  circular_slice_buffer.Push(2);
  ASSERT_TRUE(operator==
              (circular_slice_buffer.ContiguousMemoryChunks(), {{0, 1}, {2}}));
  for (SizeType i{3}; i < kCapacity; ++i) {
    circular_slice_buffer.Push(i);
  }
  ASSERT_TRUE(operator==(circular_slice_buffer.ContiguousMemoryChunks(),
                         {{0, 1}, {2, 3}, {4, 5}, {6, 7}, {8, 9}}));
  circular_slice_buffer.Push(10);
  ASSERT_TRUE(operator==(circular_slice_buffer.ContiguousMemoryChunks(),
                         {{1}, {2, 3}, {4, 5}, {6, 7}, {8, 9}, {10}}));
  circular_slice_buffer.Push(11);
  ASSERT_TRUE(operator==(circular_slice_buffer.ContiguousMemoryChunks(),
                         {{2, 3}, {4, 5}, {6, 7}, {8, 9}, {10, 11}}));
  for (SizeType i{12}; i < 2 * kCapacity; ++i) {
    circular_slice_buffer.Push(i);
  }
  ASSERT_TRUE(operator==(circular_slice_buffer.ContiguousMemoryChunks(),
                         {{10, 11}, {12, 13}, {14, 15}, {16, 17}, {18, 19}}));
}

TEST(CircularSliceBuffer, ContiguousMemoryChunksFlushWhenEmpty) {  // NOLINT
  const typename Pool::Options options{.max_slice_capacity = 2,
                                       .capacity = kCapacity};
  Pool pool{options};
  SizeType flush_count{0};
  const auto flush_handler = [&](std::vector<OwnedSlicePtr>&& slices) {
    ++flush_count;
    ASSERT_TRUE(&slices == &slices);  // TODO remove
    switch (flush_count) {
      case 1:
        // ASSERT_TRUE(operator==(circular_slice_buffer.ContiguousMemoryChunks(),
        //                        {{0, 1}, {2, 3}, {4, 5}, {6, 7}, {8}}));
        break;
      case 2:
        // ASSERT_TRUE(operator==
        //             (circular_slice_buffer.ContiguousMemoryChunks(),
        //              {{10, 11}, {12, 13}, {14, 15}, {16, 17}, {18, 19}}));
        break;
      default:
        FAIL() << "Expected 2 flushes, got " << flush_count << '.';
    }
  };
  const typename CircularSliceBuffer::Options circular_slice_buffer_options{
      .flush_handler = flush_handler,
      .buffer_slice_pool = &pool,
      .flush_when_full = true};
  CircularSliceBuffer circular_slice_buffer{circular_slice_buffer_options};

  ASSERT_TRUE(circular_slice_buffer.ContiguousMemoryChunks().empty());
  ASSERT_TRUE(operator==(circular_slice_buffer.ContiguousMemoryChunks(), {}));
  for (SizeType i{0}; i < kCapacity - 1; ++i) {
    circular_slice_buffer.Push(i);
    ASSERT_FALSE(circular_slice_buffer.ContiguousMemoryChunks().empty());
  }
  ASSERT_TRUE(operator==(circular_slice_buffer.ContiguousMemoryChunks(),
                         {{0, 1}, {2, 3}, {4, 5}, {6, 7}, {8}}));
  ASSERT_EQ(flush_count, 0);
  circular_slice_buffer.Push(9);
  ASSERT_EQ(flush_count, 1);
  ASSERT_TRUE(circular_slice_buffer.Empty());
  ASSERT_TRUE(circular_slice_buffer.ContiguousMemoryChunks().empty());
  ASSERT_TRUE(operator==(circular_slice_buffer.ContiguousMemoryChunks(), {}));
  circular_slice_buffer.Push(10);
  Print(circular_slice_buffer.ContiguousMemoryChunks());
  ASSERT_TRUE(operator==
              (circular_slice_buffer.ContiguousMemoryChunks(), {{10}}));
  for (SizeType i{11}; i < 2 * kCapacity - 1; ++i) {
    circular_slice_buffer.Push(i);
    ASSERT_FALSE(circular_slice_buffer.ContiguousMemoryChunks().empty());
  }
  ASSERT_TRUE(operator==(circular_slice_buffer.ContiguousMemoryChunks(),
                         {{10, 11}, {12, 13}, {14, 15}, {16, 17}, {18}}));
  ASSERT_EQ(flush_count, 1);
  circular_slice_buffer.Push(2 * kCapacity - 1);
  ASSERT_EQ(flush_count, 2);
  ASSERT_TRUE(circular_slice_buffer.Empty());
  ASSERT_TRUE(circular_slice_buffer.ContiguousMemoryChunks().empty());
}

}  // namespace
